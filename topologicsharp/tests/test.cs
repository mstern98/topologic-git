using System;
using System.IO;


using static topologicsharp;
using static graph;
using static SNAPSHOT;
using static MEM_OPTION;
using static VERBOSITY;
using static STATES;
using static CONTEXT; 
public class Test{
	public static void Main(String[] args){
		graph g = new graph(-1, (uint)START_STOP, 100, EDGES | FUNCTIONS, SWITCH, CONTINUE);
		Console.WriteLine("SUCCESS");

		
	}
}
