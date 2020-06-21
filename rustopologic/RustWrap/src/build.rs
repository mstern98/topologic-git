fn main(){
    pkg_config::Config::new()
        .atleast_version("1.0")
        .probe("z")
        .unwrap();

    let src=[
        "../../../src/*.c"
    ];
    let include=[
        "../../../include/*.h"
    ];
    let mut builder = cc::Build::new();
    let build = builder
        .files(src.iter())
        .include("../../../include/*")
        .flag("-Wall")
        .flag("-Werror")
        .flag("-g")
        .flag("-lpthread")
        .flag("-pthread")
        .flag("-L. -ltopologic -lfl -fPIC")
        .define("USE_ZLIB", None);

    build.compile("rustopologic");
}
