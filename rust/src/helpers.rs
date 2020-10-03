/*
 * Some often used helper functions
 */

use std::ffi::{ CStr, CString, OsStr };
use std::path::Path;
use std::ptr::slice_from_raw_parts;

pub fn str_to_c_char_ptr(s: &str) -> *mut cty::c_char {
    let c_str = CString::new(s.as_bytes()).unwrap_or_default();
    c_str.into_raw() as *mut cty::c_char
}

pub unsafe fn char_ptr_to_string(s: *const cty::c_char) -> String {
    String::from(CStr::from_ptr(s).to_str().unwrap())
}

pub fn get_extension(filename: &str) -> Option<String> {
    let mut v: Vec<char> = filename.chars().collect();
    let len = v.len();
    let pos = v.iter().position(|&c| c == '.');

    if  len < 2 ||
        v[0] == '.' && v.iter().filter(|&c| *c == '.').count() < 2 ||
        pos.is_none() {
        return None
    }

    let mut s = String::new();
    for i in pos.unwrap()+1..len {
        s.push(v[i]);
    }

    Some(s)
}

pub fn fl_c_array_to_str(fixed_arr: &[cty::c_char; 256]) -> String {
    let ptr_arr = fixed_arr.as_ptr();
    assert!(!ptr_arr.is_null());
    let c_str: &CStr = unsafe { CStr::from_ptr(ptr_arr) };
    
    String::from(c_str.to_str().unwrap())
}

pub unsafe fn double_i8_ptr_to_vec(ptr: *mut *mut i8, ptr_len: usize) -> Vec<String> {
    let pv = slice_from_raw_parts(ptr, ptr_len);
    let mut ptr_vec: Vec<String> = Vec::new();

    for i in 0..ptr_len {
        let elem = (*pv)[i];
        ptr_vec.push(char_ptr_to_string(elem));
    }

    ptr_vec
}

fn min(a: usize, b: usize) -> usize {
    if a > b { return b }

    a
}

pub fn strncmp(a: &str, b: &str) -> cty::c_int {
    let a: Vec<char> = a.chars().collect();
    let b: Vec<char> = b.chars().collect();

    let mut n = min(a.len(), b.len());

    while n > 0 {
        n -= 1;
        if a[n] < b[n] { return - 1 }
        else if a[n] > b[n] { return 1 }
    }

    0
}

pub fn strncmp_fl(a: &str, b: &str, mut n: usize) -> cty::c_int {
    let a: Vec<char> = a.chars().collect();
    let b: Vec<char> = b.chars().collect();

    while n > 0 {
        n -= 1;
        if (a[n] as cty::c_uchar) < (b[n] as cty::c_uchar) { return - 1 }
        else if (a[n] as cty::c_uchar) > (b[n] as cty::c_uchar) { return 1 }
    }

    0
}

pub fn match_position(s: &str, v: &Vec<String>) -> cty::c_int {
    match v.iter().position(|r| r == s) {
        Some(val) => val as i32,
        None => -1
    }
}

// not needed yet
pub fn is_fnmatch(filename: &str) -> bool {
    let fnmatch_chars: Vec<char> = vec![
        '!',
        '*',
        '?',
        '[',
        ']',
        '\0'
    ];

    return fnmatch_chars.into_iter().any(|c| filename.contains(c));
}

fn is_match(sub: &Vec<char>, s: &Vec<char>) -> bool {
    let mut result = true;
    for i in 0..sub.len() {
        if sub[i] != s[i] {
            result = false;
        }
    }

    result
}

pub fn get_position_in_string(sub: &str, s: &str) -> i32 {
    let mut pos = -1;
    let mut rel_pos = 0;

    let longstring = String::from(s);
    let mut longstring_vec: Vec<char> = longstring.chars().collect();
    let substring_vec: Vec<char> = sub.chars().collect();

    while longstring_vec.len() >= substring_vec.len() {
        if is_match(&substring_vec, &longstring_vec) {
            pos = rel_pos;
            break;
        } else {
            longstring_vec.remove(0);
            rel_pos += 1;
        }
    }

    pos
}
