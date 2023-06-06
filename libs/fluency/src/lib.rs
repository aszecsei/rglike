#![allow(clippy::missing_safety_doc)]

use std::str::FromStr;

use fluent::{
    bundle::FluentBundle, FluentArgs, FluentAttribute, FluentMessage, FluentResource, FluentValue,
};
use intl_memoizer::concurrent::IntlLangMemoizer;
use std::ffi;
use unic_langid::LanguageIdentifier;

/// A collection of localization messages for a single locale, which are meant
/// to be used together in a single view, widget or any other UI abstraction.
pub struct FluencyBundle(FluentBundle<FluentResource, IntlLangMemoizer>);
impl From<FluentBundle<FluentResource, IntlLangMemoizer>> for FluencyBundle {
    fn from(value: FluentBundle<FluentResource, IntlLangMemoizer>) -> Self {
        FluencyBundle(value)
    }
}

pub struct FluencyMessage<'a>(FluentMessage<'a>);
impl<'a> From<FluentMessage<'a>> for FluencyMessage<'a> {
    fn from(value: FluentMessage<'a>) -> Self {
        FluencyMessage(value)
    }
}

pub struct FluencyAttribute<'a>(FluentAttribute<'a>);
impl<'a> From<FluentAttribute<'a>> for FluencyAttribute<'a> {
    fn from(value: FluentAttribute<'a>) -> Self {
        FluencyAttribute(value)
    }
}

pub struct FluencyArgs<'a>(FluentArgs<'a>);
impl<'a> From<FluentArgs<'a>> for FluencyArgs<'a> {
    fn from(value: FluentArgs<'a>) -> Self {
        FluencyArgs(value)
    }
}

#[repr(C)]
#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub enum FluencyResult {
    Ok = 0,
    InvalidLocale = 1,
    InvalidUnicode = 2,
    NullPointer = 3,
    MissingValue = 4,
}

pub extern "C" fn is_ok(result: FluencyResult) -> bool {
    result == FluencyResult::Ok
}
pub extern "C" fn is_error(result: FluencyResult) -> bool {
    result != FluencyResult::Ok
}
#[inline]
fn rust_string_to_c(rust_string: impl Into<String>) -> *mut ffi::c_char {
    let cstr = match ffi::CString::new(rust_string.into()) {
        Ok(c) => c,
        Err(_) => return std::ptr::null_mut(),
    };
    cstr.into_raw()
}

#[inline]
fn c_string_to_rust<'a>(cstring: *const ffi::c_char) -> Result<&'a str, FluencyResult> {
    if cstring.is_null() {
        return Err(FluencyResult::NullPointer);
    }

    let raw = unsafe { ffi::CStr::from_ptr(cstring) };
    match raw.to_str() {
        Ok(s) => Ok(s),
        Err(_) => Err(FluencyResult::InvalidUnicode),
    }
}

/// Destroys a string created by Fluency.
#[no_mangle]
pub unsafe extern "C" fn destroy_string(cstring: *mut ffi::c_char) {
    if !cstring.is_null() {
        drop(ffi::CString::from_raw(cstring))
    }
}

/// Constructs a FluencyBundle. The locale should be the language this bundle represents, and will be used to determine the correct plural rules for this bundle.
#[no_mangle]
pub unsafe extern "C" fn create_bundle(
    locale: *const ffi::c_char,
    bundle: &mut *mut FluencyBundle,
) -> FluencyResult {
    let locale_str = match c_string_to_rust(locale) {
        Err(e) => return e,
        Ok(s) => s,
    };

    let locale = match LanguageIdentifier::from_str(locale_str) {
        Ok(l) => l,
        Err(_) => return FluencyResult::InvalidLocale,
    };

    let fbundle: FluentBundle<FluentResource, IntlLangMemoizer> =
        FluentBundle::new_concurrent(vec![locale]);
    *bundle = Box::into_raw(Box::new(fbundle.into()));

    FluencyResult::Ok
}

/// Destroys a FluencyBundle.
#[no_mangle]
pub unsafe extern "C" fn destroy_bundle(bundle: *mut FluencyBundle) {
    if !bundle.is_null() {
        drop(Box::from_raw(bundle));
    }
}

fn vec_to_c_arr<T>(mut v: Vec<T>) -> (*mut T, usize) {
    v.shrink_to_fit();
    assert!(v.len() == v.capacity());
    let ptr = v.as_mut_ptr();
    let len = v.len();
    std::mem::forget(v);
    (ptr, len)
}

/// Destroys an array of strings created by Fluency.
#[no_mangle]
pub unsafe extern "C" fn destroy_str_array(arr: *mut *mut ffi::c_char, len: usize) {
    if !arr.is_null() {
        let v = Vec::from_raw_parts(arr, len, len);
        for cstr in v {
            destroy_string(cstr);
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn add_resource(
    bundle: *mut FluencyBundle,
    string: *const ffi::c_char,
    errors: &mut *mut *mut ffi::c_char,
    errors_len: &mut usize,
) -> FluencyResult {
    if bundle.is_null() {
        return FluencyResult::NullPointer;
    }

    let ftl_string = match c_string_to_rust(string) {
        Err(e) => return e,
        Ok(s) => s,
    };

    let mut error_vec = vec![];

    let parsed_resource = match FluentResource::try_new(ftl_string.to_owned()) {
        Ok(r) => r,
        Err(err) => {
            error_vec = err
                .1
                .into_iter()
                .map(|e| rust_string_to_c(format!("{} ({}:{})", e.kind, e.pos.start, e.pos.end)))
                .collect();
            err.0
        }
    };

    let bundle = &mut *bundle;

    if let Err(e) = bundle.0.add_resource(parsed_resource) {
        error_vec.extend(
            e.into_iter()
                .map(|fe| rust_string_to_c(format!("{:?}", fe))),
        );
    }

    if !error_vec.is_empty() {
        let (e, l) = vec_to_c_arr(error_vec);
        *errors = e;
        *errors_len = l;
    } else {
        *errors_len = 0;
    }

    FluencyResult::Ok
}

#[no_mangle]
pub unsafe extern "C" fn add_resource_overriding(
    bundle: *mut FluencyBundle,
    string: *const ffi::c_char,
    errors: &mut *mut *mut ffi::c_char,
    errors_len: &mut usize,
) -> FluencyResult {
    if bundle.is_null() {
        return FluencyResult::NullPointer;
    }

    let ftl_string = match c_string_to_rust(string) {
        Err(e) => return e,
        Ok(s) => s,
    };

    let mut error_vec = vec![];

    let parsed_resource = match FluentResource::try_new(ftl_string.to_owned()) {
        Ok(r) => r,
        Err(err) => {
            error_vec = err
                .1
                .into_iter()
                .map(|e| rust_string_to_c(format!("{} ({}:{})", e.kind, e.pos.start, e.pos.end)))
                .collect();
            err.0
        }
    };

    let bundle = &mut *bundle;

    bundle.0.add_resource_overriding(parsed_resource);

    if !error_vec.is_empty() {
        let (e, l) = vec_to_c_arr(error_vec);
        *errors = e;
        *errors_len = l;
    } else {
        *errors_len = 0;
    }

    FluencyResult::Ok
}

#[no_mangle]
pub unsafe extern "C" fn set_use_isolating(
    bundle: *mut FluencyBundle,
    value: bool,
) -> FluencyResult {
    if bundle.is_null() {
        return FluencyResult::NullPointer;
    }
    let bundle = &mut *bundle;
    bundle.0.set_use_isolating(value);
    FluencyResult::Ok
}

// TODO: Set transform
// TODO: Set formatter

#[no_mangle]
pub unsafe extern "C" fn has_message(
    bundle: *mut FluencyBundle,
    id: *const ffi::c_char,
    result: &mut bool,
) -> FluencyResult {
    if bundle.is_null() {
        return FluencyResult::NullPointer;
    }

    let id_string = match c_string_to_rust(id) {
        Err(e) => return e,
        Ok(s) => s,
    };

    let bundle = &mut *bundle;
    *result = bundle.0.has_message(id_string);
    FluencyResult::Ok
}

#[no_mangle]
pub unsafe extern "C" fn get_message(
    bundle: *mut FluencyBundle,
    id: *const ffi::c_char,
    message: &mut *mut FluencyMessage,
) -> FluencyResult {
    if bundle.is_null() {
        return FluencyResult::NullPointer;
    }

    let id_string = match c_string_to_rust(id) {
        Err(e) => return e,
        Ok(s) => s,
    };

    let bundle = &mut *bundle;
    let msg = bundle.0.get_message(id_string);

    *message = match msg {
        None => std::ptr::null_mut(),
        Some(msg) => Box::into_raw(Box::new(msg.into())),
    };

    FluencyResult::Ok
}

#[no_mangle]
pub unsafe extern "C" fn destroy_message(message: *mut FluencyMessage) {
    if !message.is_null() {
        drop(Box::from_raw(message));
    }
}

#[no_mangle]
pub unsafe extern "C" fn create_args(args: &mut *mut FluencyArgs) {
    *args = Box::into_raw(Box::new(FluentArgs::new().into()))
}

#[no_mangle]
pub unsafe extern "C" fn create_args_with_capacity(args: &mut *mut FluencyArgs, capacity: usize) {
    *args = Box::into_raw(Box::new(FluentArgs::with_capacity(capacity).into()))
}

#[no_mangle]
pub unsafe extern "C" fn destroy_args(args: *mut FluencyArgs) {
    if !args.is_null() {
        drop(Box::from_raw(args));
    }
}

#[no_mangle]
pub unsafe extern "C" fn set_arg_try_number(
    args: *mut FluencyArgs,
    key: *const ffi::c_char,
    value: *const ffi::c_char,
) -> FluencyResult {
    if args.is_null() {
        return FluencyResult::NullPointer;
    }
    let key_string = match c_string_to_rust(key) {
        Err(e) => return e,
        Ok(s) => s,
    };
    let value_string = match c_string_to_rust(value) {
        Err(e) => return e,
        Ok(s) => s,
    };

    let value = FluentValue::try_number(value_string);
    let args = &mut *args;
    args.0.set(key_string, value);

    FluencyResult::Ok
}

#[no_mangle]
pub unsafe extern "C" fn set_arg_string(
    args: *mut FluencyArgs,
    key: *const ffi::c_char,
    value: *const ffi::c_char,
) -> FluencyResult {
    if args.is_null() {
        return FluencyResult::NullPointer;
    }
    let key_string = match c_string_to_rust(key) {
        Err(e) => return e,
        Ok(s) => s,
    };
    let value_string = match c_string_to_rust(value) {
        Err(e) => return e,
        Ok(s) => s,
    };
    
    let args = &mut *args;
    args.0.set(key_string, value_string);

    FluencyResult::Ok
}
#[no_mangle]
pub unsafe extern "C" fn set_arg_i8(
    args: *mut FluencyArgs,
    key: *const ffi::c_char,
    value: i8,
) -> FluencyResult {
    if args.is_null() {
        return FluencyResult::NullPointer;
    }
    let key_string = match c_string_to_rust(key) {
        Err(e) => return e,
        Ok(s) => s,
    };

    let args = &mut *args;
    args.0.set(key_string, value);

    FluencyResult::Ok
}
#[no_mangle]
pub unsafe extern "C" fn set_arg_i16(
    args: *mut FluencyArgs,
    key: *const ffi::c_char,
    value: i16,
) -> FluencyResult {
    if args.is_null() {
        return FluencyResult::NullPointer;
    }
    let key_string = match c_string_to_rust(key) {
        Err(e) => return e,
        Ok(s) => s,
    };

    let args = &mut *args;
    args.0.set(key_string, value);

    FluencyResult::Ok
}
#[no_mangle]
pub unsafe extern "C" fn set_arg_i32(
    args: *mut FluencyArgs,
    key: *const ffi::c_char,
    value: i32,
) -> FluencyResult {
    if args.is_null() {
        return FluencyResult::NullPointer;
    }
    let key_string = match c_string_to_rust(key) {
        Err(e) => return e,
        Ok(s) => s,
    };

    let args = &mut *args;
    args.0.set(key_string, value);

    FluencyResult::Ok
}
#[no_mangle]
pub unsafe extern "C" fn set_arg_i64(
    args: *mut FluencyArgs,
    key: *const ffi::c_char,
    value: i64,
) -> FluencyResult {
    if args.is_null() {
        return FluencyResult::NullPointer;
    }
    let key_string = match c_string_to_rust(key) {
        Err(e) => return e,
        Ok(s) => s,
    };

    let args = &mut *args;
    args.0.set(key_string, value);

    FluencyResult::Ok
}

#[no_mangle]
pub unsafe extern "C" fn set_arg_u8(
    args: *mut FluencyArgs,
    key: *const ffi::c_char,
    value: u8,
) -> FluencyResult {
    if args.is_null() {
        return FluencyResult::NullPointer;
    }
    let key_string = match c_string_to_rust(key) {
        Err(e) => return e,
        Ok(s) => s,
    };

    let args = &mut *args;
    args.0.set(key_string, value);

    FluencyResult::Ok
}
#[no_mangle]
pub unsafe extern "C" fn set_arg_u16(
    args: *mut FluencyArgs,
    key: *const ffi::c_char,
    value: u16,
) -> FluencyResult {
    if args.is_null() {
        return FluencyResult::NullPointer;
    }
    let key_string = match c_string_to_rust(key) {
        Err(e) => return e,
        Ok(s) => s,
    };

    let args = &mut *args;
    args.0.set(key_string, value);

    FluencyResult::Ok
}
#[no_mangle]
pub unsafe extern "C" fn set_arg_u32(
    args: *mut FluencyArgs,
    key: *const ffi::c_char,
    value: u32,
) -> FluencyResult {
    if args.is_null() {
        return FluencyResult::NullPointer;
    }
    let key_string = match c_string_to_rust(key) {
        Err(e) => return e,
        Ok(s) => s,
    };

    let args = &mut *args;
    args.0.set(key_string, value);

    FluencyResult::Ok
}
#[no_mangle]
pub unsafe extern "C" fn set_arg_u64(
    args: *mut FluencyArgs,
    key: *const ffi::c_char,
    value: u64,
) -> FluencyResult {
    if args.is_null() {
        return FluencyResult::NullPointer;
    }
    let key_string = match c_string_to_rust(key) {
        Err(e) => return e,
        Ok(s) => s,
    };

    let args = &mut *args;
    args.0.set(key_string, value);

    FluencyResult::Ok
}

#[no_mangle]
pub unsafe extern "C" fn set_arg_f32(
    args: *mut FluencyArgs,
    key: *const ffi::c_char,
    value: f32,
) -> FluencyResult {
    if args.is_null() {
        return FluencyResult::NullPointer;
    }
    let key_string = match c_string_to_rust(key) {
        Err(e) => return e,
        Ok(s) => s,
    };

    let args = &mut *args;
    args.0.set(key_string, value);

    FluencyResult::Ok
}
#[no_mangle]
pub unsafe extern "C" fn set_arg_f64(
    args: *mut FluencyArgs,
    key: *const ffi::c_char,
    value: f64,
) -> FluencyResult {
    if args.is_null() {
        return FluencyResult::NullPointer;
    }
    let key_string = match c_string_to_rust(key) {
        Err(e) => return e,
        Ok(s) => s,
    };

    let args = &mut *args;
    args.0.set(key_string, value);

    FluencyResult::Ok
}

#[no_mangle]
pub unsafe extern "C" fn get_attribute<'a>(
    message: *mut FluencyMessage<'a>,
    key: *const ffi::c_char,
    attribute: &mut *mut FluencyAttribute<'a>,
) -> FluencyResult {
    if message.is_null() {
        return FluencyResult::NullPointer;
    }
    let key_str = match c_string_to_rust(key) {
        Err(e) => return e,
        Ok(s) => s,
    };
    let message = &mut *message;

    *attribute = match message.0.get_attribute(key_str) {
        None => std::ptr::null_mut(),
        Some(attr) => Box::into_raw(Box::new(attr.into())),
    };

    FluencyResult::Ok
}

#[no_mangle]
pub unsafe extern "C" fn destroy_attribute(attribute: *mut FluencyAttribute) {
    if !attribute.is_null() {
        drop(Box::from_raw(attribute));
    }
}

#[no_mangle]
pub unsafe extern "C" fn get_attributes<'a>(
    message: *mut FluencyMessage<'a>,
    attributes: &mut *mut FluencyAttribute<'a>,
    attributes_len: &mut usize,
) -> FluencyResult {
    if message.is_null() {
        return FluencyResult::NullPointer;
    }
    let message = &mut *message;
    let v: Vec<FluencyAttribute> = message.0.attributes().map(FluencyAttribute::from).collect();

    (*attributes, *attributes_len) = vec_to_c_arr(v);

    FluencyResult::Ok
}

#[no_mangle]
pub unsafe extern "C" fn destroy_attributes(attributes: *mut FluencyAttribute, len: usize) {
    if !attributes.is_null() {
        let v = Vec::from_raw_parts(attributes, len, len);
        drop(v);
    }
}

#[no_mangle]
pub unsafe extern "C" fn get_attribute_id(
    attribute: *mut FluencyAttribute,
    id: &mut *mut ffi::c_char,
) -> FluencyResult {
    if attribute.is_null() {
        return FluencyResult::NullPointer;
    }
    let attribute = &mut *attribute;
    *id = rust_string_to_c(attribute.0.id());

    FluencyResult::Ok
}

#[no_mangle]
pub unsafe extern "C" fn format_message(
    bundle: *mut FluencyBundle,
    message: *const FluencyMessage,
    args: *const FluencyArgs,
    result: &mut *mut ffi::c_char,
    errors: &mut *mut *mut ffi::c_char,
    errors_len: &mut usize,
) -> FluencyResult {
    if bundle.is_null() {
        return FluencyResult::NullPointer;
    }
    if message.is_null() {
        return FluencyResult::NullPointer;
    }
    let bundle = &mut *bundle;
    let message = &*message;
    let args = if args.is_null() { None } else { Some(&*args) };
    let mut error_vec = vec![];

    let pattern = match message.0.value() {
        None => return FluencyResult::MissingValue,
        Some(p) => p,
    };

    let formatted = bundle.0.format_pattern(pattern, args.map(|a| &a.0), &mut error_vec);
    *result = rust_string_to_c(formatted);

    if !error_vec.is_empty() {
        let error_vec: Vec<_> = error_vec.into_iter().map(|fe| rust_string_to_c(format!("{:?}", fe))).collect();
        (*errors, *errors_len) = vec_to_c_arr(error_vec);
    } else {
        *errors_len = 0;
    }

    FluencyResult::Ok
}

#[no_mangle]
pub unsafe extern "C" fn format_attribute(
    bundle: *mut FluencyBundle,
    attribute: *const FluencyAttribute,
    args: *const FluencyArgs,
    result: &mut *mut ffi::c_char,
    errors: &mut *mut *mut ffi::c_char,
    errors_len: &mut usize,
) -> FluencyResult {
    if bundle.is_null() {
        return FluencyResult::NullPointer;
    }
    if attribute.is_null() {
        return FluencyResult::NullPointer;
    }
    let bundle = &mut *bundle;
    let attribute = &*attribute;
    let args = if args.is_null() { None } else { Some(&*args) };
    let mut error_vec = vec![];

    let pattern = attribute.0.value();

    let formatted = bundle.0.format_pattern(pattern, args.map(|a| &a.0), &mut error_vec);
    *result = rust_string_to_c(formatted);

    if !error_vec.is_empty() {
        let error_vec: Vec<_> = error_vec.into_iter().map(|fe| rust_string_to_c(format!("{:?}", fe))).collect();
        (*errors, *errors_len) = vec_to_c_arr(error_vec);
    } else {
        *errors_len = 0;
    }

    FluencyResult::Ok
}
