fn main(){
    //println!("cargo:rustc-link-search=lib");    

    let src=[
        "src/AVL.c",
        "src/edge.c",
        "src/graph.c",
        "src/request.c",
        "src/topologic.c",
        "src/topologic_json.c",
        "src/vertex.c",
        "src/windows_wrap.c"
    ];
    let _include=[
        "../../../include"
    ];
    let mut builder = cc::Build::new();
    let build = builder
        .files(src.iter())
        .include(_include[0])
        .flag("-Wall")
        .flag("-Werror")
        .flag("-g")
        .flag("-lpthread")
        .flag("-pthread")
        .flag("-L. -ltopologic -lfl -fPIC")
        .define("USE_ZLIB", None);

    build.compile("rustTopologic");
}
