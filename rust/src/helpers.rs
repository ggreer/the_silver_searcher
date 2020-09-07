/*
 * Some often used helper functions
 */

use std::ffi::{ CStr, CString, OsStr };
use std::path::Path;

pub fn str_to_c_char_ptr(s: &str) -> *mut cty::c_char {
    let c_str = CString::new(s.as_bytes()).unwrap_or_default();
    return c_str.into_raw() as *mut cty::c_char;
}

pub unsafe fn char_ptr_to_string(s: *const cty::c_char) -> String {
    return String::from(CStr::from_ptr(s).to_str().unwrap());
}

pub fn get_extension_from_filename(filename: &str) -> Option<&str> {
    Path::new(filename)
        .extension()
        .and_then(OsStr::to_str)
}

pub fn fl_c_char_ptr_to_str(fixed_arr: &[cty::c_char; 256]) -> &'static str {
    let ptr_arr = fixed_arr.as_ptr();
    assert!(!ptr_arr.is_null());
    let c_str: &CStr = unsafe { CStr::from_ptr(ptr_arr) };
    return c_str.to_str().unwrap()
}
