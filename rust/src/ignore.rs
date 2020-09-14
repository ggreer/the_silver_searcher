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
    get_extension, char_ptr_to_string, str_to_c_char_ptr, fl_c_char_ptr_to_str,
    double_i8_ptr_to_vec, strncmp, strncmp_fl, match_position, is_fnmatch,
    get_position_in_string
};

use std::ffi::{ CStr, CString };
use std::ptr::slice_from_raw_parts;
use std::mem;
use std::str;
use regex::Regex;


// placeholder until log_debug is translated
fn log_debug_rs(message: &str) {
    unsafe { log_debug(str_to_c_char_ptr(message)) };
}

unsafe fn ackmate_dir_match(dir_name: *const cty::c_char) -> cty::c_int {
    if opts.ackmate_dir_filter.is_null() {
        return 0;
    }
    /* we just care about the match, not where the matches are */
    pcre_exec(
        opts.ackmate_dir_filter, mem::MaybeUninit::uninit().assume_init(), 
        dir_name, char_ptr_to_string(dir_name).len() as i32, 0, 
        0, mem::MaybeUninit::uninit().assume_init(), 0
    )
}

fn match_regexes(pattern: Vec<String>, match_str: &str) -> bool{
    for mut name in pattern {
        if name.starts_with('*') {
            name = format!("{}{}", "[[:alpha:]]", &name);
        }
        let re = Regex::new(&name).unwrap();
        if re.is_match(match_str) {
            let message = format!("{} {} {} {}", "File", match_str, "ignored because name matches slash regex pattern", &name);
            log_debug_rs(&message);
            return false
        }
        let message = format!("{} {} {} {}", "Pattern", &name, "doesn't match slash file", match_str);
        log_debug_rs(&message);
    }

    true
}

fn match_static_pattern(vec: &Vec<String>, s: &str) -> bool {
    let match_pos = match_position(s, vec);
    if match_pos != -1 {
        let message = format!("{} {} {} {}", "File", s, "ignored because name matches static pattern", vec[match_pos as usize]);
        log_debug_rs(&message);
        return true
    }

    false
}

fn match_slash_filename(vec: &Vec<String>, s: &str) -> bool {
    for v in vec {
        let pos = get_position_in_string(v, s);
        if pos != -1 {
            let longstring = String::from(s);
            let mut longstring_vec: Vec<char> = longstring.chars().collect();
            let long_len = longstring_vec.len() as usize;

            let substring: Vec<char> = v.chars().collect();
            let sub_len = substring.len();

            let mut one_before_pos = '?';
            if pos > 1 {
                one_before_pos = longstring_vec[pos as usize - 1];
            }

            let mut string_vec_at_pos: Vec<char> = Vec::new();
            for i in pos as usize..long_len {
                string_vec_at_pos.push(longstring_vec[i]);
            }

            let string_at_pos: String = string_vec_at_pos.iter().collect();
            if string_at_pos == s || one_before_pos == '/' {
                for i in 0..sub_len {
                    string_vec_at_pos.remove(0);
                }
                if string_vec_at_pos[0] == '\0' || string_vec_at_pos[0] == '/' {
                    let message = format!("{} {} {} {}", "File", s, "ignored because name matches static pattern", v);
                    log_debug_rs(&message);
                    return true
                }
            }
        }
        let message = format!("{} {} {} {}", "Pattern", v, "doesn't match path", s);
        log_debug_rs(&message);
    }

    false
}

/* This is the hottest code in Ag. 10-15% of all execution time is spent here */
unsafe fn path_ignore_search(ig: *const ignores, path: &str, filename: &str) -> bool {
    // Some convencience defines
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

    if match_static_pattern(&names_vec, &filename) { return true };

    let mut path_str = String::from(path);
    if path_str.starts_with('.') {
        path_str.remove(0);
    }

    let mut temp = format!("{}/{}", &path_str, &filename);

    // strip leading slash like abs_path
    if temp.starts_with('/') {
        temp.remove(0);
    }

    let mut slash_filename = String::from(&temp);

    if strncmp_fl(&slash_filename, &char_ptr_to_string(abs_path), abs_path_len) == 0 {
        for i in 0..abs_path_len {
            if slash_filename.len() > 0 {
                slash_filename.remove(0);
            }
        }

        if slash_filename.len() > 0 && slash_filename.starts_with('/') {
            slash_filename.remove(0);
        }

        if  match_static_pattern(&names_vec, &slash_filename) ||
            match_static_pattern(&slash_names_vec, &slash_filename) ||
            match_slash_filename(&names_vec, &slash_filename) {
            return true 
        }

        if !match_regexes(slash_regexes_vec, &slash_filename) { return false }
    }

    if !match_regexes(invert_regexes_vec, &filename) { return false }
    if !match_regexes(regexes_vec, &filename) { return true }

    ackmate_dir_match(str_to_c_char_ptr(&temp)) == 1
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

    false
}

fn is_unwanted_symlink(filename: &str, d_type: cty::c_uchar) -> bool {
    if unsafe { opts.follow_symlinks  } == 0 && d_type == DT_LNK {
        let message = format!("{} {} {}", "File", filename, "ignored becaused it's a symlink");
        log_debug_rs(&message);
        return true
    }

    false
}

fn is_fifo(filename: &str, d_type: cty::c_uchar) -> bool {
    if d_type == DT_FIFO {
        let message = format!("{} {} {}", "File", filename, "ignored becaused it's a named pipe or socket");
        log_debug_rs(&message);
        return true
    }

    false
}

fn check_extension(filename: &str, extensions: &Vec<String>) -> bool {
    let extension = get_extension(&filename);

    if extension.is_some() {
        let extension = extension.unwrap();
        if extensions.contains(&extension) {
            let message = format!("{} {} {} {}", "File", filename, "ignored because name matches extension", &extension);
            log_debug_rs(&message);
            return false
        }
    }

    true
}

fn check_dir(filename_vec: &Vec<char>, d_type: cty::c_uchar, path_start: &str, ig: *const ignores) -> bool {
    if d_type == DT_DIR {
        if filename_vec[&filename_vec.len() - 1] != '/' {
            let s: String = filename_vec.iter().collect();
            let temp = format!("{}/", &s);
            if unsafe { path_ignore_search(ig, path_start, &temp) } {
                return false
            }
        }
    }

    true
}

fn is_return_condition_a(filename_vec: &Vec<char>, d_type: cty::c_uchar) -> bool {
    let s: String = filename_vec.iter().collect();
    
    let cond_a = unsafe { opts.search_hidden_files == 0 } && filename_vec[0] == '.';
    let cond_b = is_evil_hardcoded(&s);
    let cond_c = is_unwanted_symlink(&s, d_type);
    let cond_d = is_fifo(&s, d_type);

    cond_a || cond_b || cond_c || cond_d
}

unsafe fn is_return_condition_b(path: *const cty::c_char, filename_vec: &Vec<char>, d_type: cty::c_uchar,
                                path_start: &str, ig: *const ignores) -> bool {
    let s: String = filename_vec.iter().collect();
    let extensions = double_i8_ptr_to_vec((*ig).extensions, (*ig).extensions_len as usize);

    let cond_a = !check_extension(&s, &extensions);
    let cond_b = path_ignore_search(ig, path_start, &s);
    let cond_c = !check_dir(&filename_vec, d_type, path_start, ig);

    cond_a || cond_b || cond_c
}

#[no_mangle]
pub unsafe extern "C" fn filename_filter(path: *const cty::c_char, dir: *const dirent, baton: *mut cty::c_void) -> cty::c_int {
    let filename: &str = fl_c_char_ptr_to_str(&(*dir).d_name);
    let mut filename_vec: Vec<char> = filename.chars().collect();

    if is_return_condition_a(&filename_vec, (*dir).d_type) { return 0 };
    
    if opts.search_all_files == 1 && opts.path_to_ignore == 0 { return 1 }

    if filename_vec[0] == '.' && filename_vec[1] == '/' {
        filename_vec.remove(0);
    }

    let scandir_baton = baton as *const scandir_baton_t;
    let path_start = (*scandir_baton).path_start;
    let mut ig = (*scandir_baton).ig;

    while !ig.is_null() {
        if is_return_condition_b(path, &filename_vec, (*dir).d_type, &char_ptr_to_string(path_start), ig) { return 0 }
        ig = (*ig).parent;
    }

    let message = format!("{} {}", filename, "not ignored");
    log_debug(str_to_c_char_ptr(&message));

    1
}
