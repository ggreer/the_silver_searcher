/*
 * ignore.c rust version
 */

use crate::bindings::{
    dirent, scandir_baton_t, ignores,
    log_debug, pcre_exec,
    opts,
};

use crate::file_types::*;

use crate::helpers::{
    get_extension_from_filename, char_ptr_to_string, str_to_c_char_ptr, fl_c_char_ptr_to_str,
    double_i8_ptr_to_vec, strncmp, strncmp_fl, match_position, is_fnmatch
};

use std::ffi::{ CStr, CString };
use std::ptr::slice_from_raw_parts;
use std::mem;
use std::str;
use regex::Regex;


unsafe fn ackmate_dir_match(dir_name: *const cty::c_char) -> cty::c_int {
    if opts.ackmate_dir_filter.is_null() {
        return 0;
    }
    /* we just care about the match, not where the matches are */
    return pcre_exec(
        opts.ackmate_dir_filter, mem::MaybeUninit::uninit().assume_init(), dir_name, 
        char_ptr_to_string(dir_name).len() as i32, 0, 0, mem::MaybeUninit::uninit().assume_init(), 0
    );
}

unsafe fn match_regexes(pattern: Vec<String>, match_str: &str) -> bool{
    for mut name in pattern {
        if name.starts_with('*') {
            name = format!("{}{}", "[[:alpha:]]", &name);
        }
        let re = Regex::new(&name).unwrap();
        if re.is_match(match_str) {
            let message = format!("{} {} {} {}", "File", match_str, "ignored because name matches slash regex pattern", &name);
            log_debug(str_to_c_char_ptr(&message));
            return false
        }
        let message = format!("{} {} {} {}", "Pattern", &name, "doesn't match slash file", match_str);
        log_debug(str_to_c_char_ptr(&message));
    }

    return true
}

unsafe fn match_static_pattern(vec: &Vec<String>, s: &str) -> bool {
    let match_pos = match_position(s, vec);
    if match_pos != -1 {
        let message = format!("{} {} {} {}", "File", s, "ignored because name matches static pattern", vec[match_pos as usize]);
        log_debug(str_to_c_char_ptr(&message));
        return true
    }

    return false
}

unsafe fn match_slash_filename(vec: &Vec<String>, s: &str) -> bool {
    for v in vec {
        if v.contains(s) {
            let leading_slash_filename = format!("/{}", s);
            let ending_filename = format!("{}\0", s);
            let ending_slash_filename = format!("{}/", s);
            if v == s || v.contains(&leading_slash_filename) || v.contains(&ending_filename) || v.contains(&ending_slash_filename) {
                let message = format!("{} {} {} {}", "File", s, "ignored because name matches static pattern", v);
                log_debug(str_to_c_char_ptr(&message));
                return true
            }
        }
        let message = format!("{} {} {} {}", "Pattern", v, "doesn't match path", s);
        log_debug(str_to_c_char_ptr(&message));            
    }

    return false;
}

/* This is the hottest code in Ag. 10-15% of all execution time is spent here */
unsafe fn path_ignore_search(ig: *const ignores, path: *const cty::c_char, filename: *const cty::c_char) -> bool {
    // Some convencience defines
    // ********************************************************************************************************* //
    let filename_str = char_ptr_to_string(filename);

    let names = (*ig).names;
    let slash_names = (*ig).slash_names;
    let regexes = (*ig).regexes; 
    let slash_regexes = (*ig).slash_regexes; 
    let invert_regexes = (*ig).invert_regexes;
    let abs_path = (*ig).abs_path;

    let names_len = (*ig).names_len as usize;
    let slash_names_len = (*ig).slash_names_len as usize;
    let regexes_len = (*ig).regexes_len as usize;
    let slash_regexes_len = (*ig).slash_regexes_len as usize;
    let invert_regexes_len = (*ig).invert_regexes_len as usize;
    let abs_path_len: usize = (*ig).abs_path_len as usize;

    let names_vec = double_i8_ptr_to_vec(names, names_len);
    let slash_names_vec = double_i8_ptr_to_vec(slash_names, slash_names_len);
    let regexes_vec = double_i8_ptr_to_vec(regexes, regexes_len);
    let slash_regexes_vec = double_i8_ptr_to_vec(slash_regexes, slash_regexes_len);
    let invert_regexes_vec = double_i8_ptr_to_vec(invert_regexes, invert_regexes_len);
    // ********************************************************************************************************* //

    if match_static_pattern(&names_vec, &filename_str) { return true };

    let mut path_str = char_ptr_to_string(path);
    if path_str.len() > 1  && path_str.starts_with('.') {
        path_str.remove(0);
    }

    let temp = format!("{}/{}", &path_str, &filename_str);
    let mut slash_filename = String::from(&temp);
    // ig->abs_path has its leading slash stripped,
    // so we have to strip the leading slash of temp as well
    if slash_filename.len() > 1 && slash_filename.starts_with('/') {
        slash_filename.remove(0);
    };

    if strncmp_fl(&slash_filename, &char_ptr_to_string(abs_path), abs_path_len) == 0 {
        for i in 0..abs_path_len {
            if slash_filename.len() > 1 {
                slash_filename.remove(0);
            }
        }

        if slash_filename.len() > 1 && slash_filename.starts_with('/') {
            slash_filename.remove(0);
        }

        if match_static_pattern(&names_vec, &slash_filename) ||
            match_static_pattern(&slash_names_vec, &slash_filename) ||
            match_slash_filename(&names_vec, &slash_filename) { 
            return true 
        }

        if !match_regexes(slash_regexes_vec, &slash_filename) { return false }
    }

    if !match_regexes(invert_regexes_vec, &filename_str) { return false }
    if !match_regexes(regexes_vec, &filename_str) { return true }

    return ackmate_dir_match(str_to_c_char_ptr(&temp)) == 1;
}


fn is_evil_hardcoded(filename: &str) -> bool {
    // some paths to always ignore
    let mut evil_hardcoded_ignore_files_rs: Vec<&str> = Vec::new();
    evil_hardcoded_ignore_files_rs.push(".");
    evil_hardcoded_ignore_files_rs.push("..");

    for file in evil_hardcoded_ignore_files_rs {
        if filename == file {
            return true
        }
    }

    return false
}

unsafe fn is_unwanted_symlink(filename: &str, d_type: cty::c_uchar) -> bool {
    if opts.follow_symlinks == 0 && d_type == DT_LNK {
        let message = format!("{} {} {}", "File", filename, "ignored becaused it's a symlink");
        log_debug(str_to_c_char_ptr(&message));
        return true
    }

    return false
}

unsafe fn is_fifo(filename: &str, d_type: cty::c_uchar) -> bool {
    if d_type == DT_FIFO {
        let message = format!("{} {} {}", "File", filename, "ignored becaused it's a named pipe or socket");
        log_debug(str_to_c_char_ptr(&message));
        return true
    }

    return false
}

unsafe fn check_extension(filename: &str, ig: *const ignores) -> bool {
    let extension = get_extension_from_filename(&filename);

    let ext_len = (*ig).extensions_len as usize;
    let extensions = slice_from_raw_parts((*ig).extensions, ext_len);
    let mut extensions_vec_c_str: Vec<CString> = Vec::new();
    for i in 0..ext_len {
        let elem = (&*extensions)[i];
        let elem_c_str = CString::from_raw(elem);
        extensions_vec_c_str.push(elem_c_str);
    }

    if extension.is_some() {
        let extension_c_str = CString::new(extension.unwrap());
        if extensions_vec_c_str.contains(&extension_c_str.unwrap()) {
            let message = format!("{} {} {} {}", "File", filename, "ignored because name matches extension", extension.unwrap());
            log_debug(str_to_c_char_ptr(&message));
            return false
        }
    }

    return true
}

unsafe fn check_dir(filename: &str, filename_vec: &Vec<char>, d_type: cty::c_uchar, path_start: *const i8, ig: *const ignores) -> bool {
    if d_type == DT_DIR {
        if filename_vec[&filename.len() - 1] != '/' {
            let temp = format!("{}/", &filename);
            if path_ignore_search(ig, path_start, str_to_c_char_ptr(&temp)) {
                return false
            }
        }
    }

    return true
}

unsafe fn is_return_condition_a(filename: &str, filename_vec: &Vec<char>, d_type: cty::c_uchar) -> bool {
    let cond_a = opts.search_hidden_files == 0 && filename_vec[0] == '.';
    let cond_b = is_evil_hardcoded(&filename);
    let cond_c = is_unwanted_symlink(&filename, d_type);
    let cond_d = is_fifo(&filename, d_type);

    return cond_a || cond_b || cond_c || cond_d
}

unsafe fn is_return_condition_b(path: *const cty::c_char, filename: &str, filename_vec: &Vec<char>,
    d_type: cty::c_uchar, path_start: *const cty::c_char, ig: *const ignores) -> bool {
    let cond_a = !check_extension(&filename, ig);
    let cond_b = path_ignore_search(ig, path_start, path);
    let cond_c = !check_dir(&filename, &filename_vec, d_type, path_start, ig);

    return cond_a || cond_b || cond_c
}

#[no_mangle]
pub unsafe extern "C" fn filename_filter(path: *const cty::c_char, dir: *const dirent, baton: *mut cty::c_void) -> cty::c_int {
    // first check whether we shall search for all files anyway
    if opts.search_all_files == 1 && opts.path_to_ignore == 0 { return 1 }

    let filename: &str = fl_c_char_ptr_to_str(&(*dir).d_name);
    // get vector of chars from dirent d_name to perform indexing on
    let mut filename_vec: Vec<char> = filename.chars().collect();

    if is_return_condition_a(&filename, &filename_vec, (*dir).d_type) { return 0 };

    // check whether d_name starts with "./"
    if filename_vec[0] == '.' && filename_vec[1] == '/' {
        filename_vec.remove(0);
    }

    let scandir_baton = baton as *const scandir_baton_t;
    let path_start = (*scandir_baton).path_start;
    let mut ig = (*scandir_baton).ig;

    while !ig.is_null() {
        if is_return_condition_b(path, &filename, &filename_vec, (*dir).d_type, path_start, ig) { return 0 }
        ig = (*ig).parent;
    }

    let message = format!("{} {}", filename, "not ignored");
    log_debug(str_to_c_char_ptr(&message));

    return 1
}
