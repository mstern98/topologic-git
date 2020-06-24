//extern crate libRustTopologic;

mod bindings;
extern crate libRustTopologic;


#[link(name="libtopologic")]
extern{
     fn graph_init(max_state_changes: i32, snapshot_timestamp: i32, max_loop: u32, lvl_verbose: u32, context: u32, mem_option: u32)-> bindings::graph ;
}

fn main(){
 
    unsafe{let graph = bindings::graph_init(-1, bindings::SNAPSHOT_START_STOP, 100, bindings::VERBOSITY_VERTICES|bindings::VERBOSITY_EDGES|bindings::VERBOSITY_FUNCTIONS|bindings::VERBOSITY_GLOBALS, bindings::CONTEXT_SINGLE, bindings::MEM_OPTION_CONTINUE);}

    
}
