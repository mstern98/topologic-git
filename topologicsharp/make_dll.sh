#!/bin/bash

echo "Creating dll's..."
sleep 1
cd topologicsharp
set -x

#mcs SWIGTYPE_p_void.cs -target:library -out:SWIGTYPE_p_void.dll
#mcs AVLNode.cs -target:library -out:AVLNode.dll
#mcs stack.cs -target:library -out:stack.dll
#mcs AVLTree.cs -target:library -out:AVLTree.dll
#mcs MEM_OPTION.cs -target:library -out:MEM_OPTION.dll
#mcs SNAPSHOT.cs -target:library -out:SNAPSHOT.dll
#mcs STATES.cs -target:library -out:STATES.dll
#mcs SWIGTYPE_CONTEXT.cs -target:library -out:SWIGTYPE_CONTEXT.dll
#mcs SWIGTYPE_REQUESTS.cs -target:library -out:SWIGTYPE_REQUESTS.dll
#mcs SWIGTYPE_p_f_p_struct_graph_p_struct_vertex_result_p_void_p_void__void.cs -target:library -out:SWIGTYPE_p_f_p_struct_graph_p_struct_vertex_result_p_void_p_void__void.dll
#mcs SWIGTYPE_p_f_p_void__void.cs -target:library -out:SWIGTYPE_p_f_p_void__void.dll
#mcs SWIGTYPE_p_f_p_void_p_void_q_const__p_q_const__void__int.cs -target:library -out:SWIGTYPE_p_f_p_void_p_void_q_const__p_q_const__void__int.dll
#mcs SWIGTYPE_p_int.cs -target:library -out:SWIGTYPE_p_int.dll
#mcs SWIGTYPE_p_p_edge.cs -target:library -out:SWIGTYPE_p_p_edge.dll
#mcs SWIGTYPE_p_p_vertex_result.cs -target:library -out:SWIGTYPE_p_p_vertex_result.dll
#mcs SWIGTYPE_p_p_void.cs -target:library -out:SWIGTYPE_p_p_void.dll
#mcs SWIGTYPE_p_phread_cond_t.cs -target:library -out:SWIGTYPE_p_phread_cond_t.dll
#mcs SWIGTYPE_p_phread_mutex_t.cs -target:library -out:SWIGTYPE_p_phread_mutex_t.dll
#mcs SWIGTYPE_p_request.cs -target:library -out:SWIGTYPE_p_request.dll
#mcs SWIGTYPE_p_sig_atomic_t.cs -target:library -out:SWIGTYPE_p_sig_atomic_t.dll
#mcs SWIGTYPE_p_void.cs -target:library -out:SWIGTYPE_p_void.dll
#mcs VERBOSITY.cs -target:library -out:VERBOSITY.dll
#mcs vertex.cs -target:library -out:vertex.dll
#mcs graph.cs -target:library -out:graph.dll





for i in * : 
do
	if [[ "$i" == *.cs ]] 
	then	
		echo "filename: $i"
		j=$(echo $i | sed 's/\.cs//g')
		#echo $j
		mcs $i -target:library -out:$j.dll
	fi
done

