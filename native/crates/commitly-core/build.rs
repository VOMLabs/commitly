fn main() {
    cxx_build::bridge("src/ffi.rs")
        .std("c++20")
        .compile("commitly-core-cxx");

    println!("cargo:rerun-if-changed=src/ffi.rs");
}
