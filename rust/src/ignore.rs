/*
 * ignore.c rust version
 */

use crate::bindings::{
    dirent, scandir_baton_t, ignores,
    path_ignore_search, ag_asprintf, log_debug,
    opts,
};

use crate::file_types::*;

use crate::helpers::{
    get_extension_from_filename, str_to_c_char_ptr, fl_c_char_ptr_to_str
};

use std::ffi::{ CStr, CString };
use std::ptr::slice_from_raw_parts;
use std::mem;
use std::str;


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
            let mut zero_u8: i8 = 0;
            let mut zero_u8_ptr: *mut i8 = &mut zero_u8;
            let temp: *mut *mut i8 = &mut zero_u8_ptr;
            ag_asprintf(temp, "%s/".as_ptr() as *const i8, &filename);
            let rv = path_ignore_search(ig, path_start, (*temp));
            if (rv > 0) {
                return false
            }
        }
    }

    return true
}

unsafe fn is_return_condition_a(filename: &str, filename_vec: &Vec<char>, d_type: cty::c_uchar) -> bool {
    let cond_a = opts.search_hidden_files == 0 && filename_vec[0] == '.';
    let cond_b = is_evil_hardcoded(&filename);
    let cond_c =  is_unwanted_symlink(&filename, d_type);
    let cond_d =  is_fifo(&filename, d_type);

    return cond_a || cond_b || cond_c || cond_d
}

unsafe fn is_return_condition_b(path: *const cty::c_char, filename: &str, filename_vec: &Vec<char>,
    d_type: cty::c_uchar, path_start: *const i8, ig: *const ignores) -> bool {
    let cond_a = !check_extension(&filename, ig);
    let cond_b = path_ignore_search(ig, path_start, path) == 1;
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
