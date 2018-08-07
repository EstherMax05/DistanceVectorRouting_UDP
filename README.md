# DistanceVectorRouting_UDP
==================
Build Instructions
==================
To build the program, run "gcc -g -o stp C_file" within assignment root directory.


==================
Usage Instructions
==================
To run the program, run "./stp ConfigurationFile CurrentNode_Name" in the root directory after compiling
e.g "./span B.txt B"

=====
Notes
=====

Limitations:  

The program provides the DVR over UDP. However, it does not attempt to handle the count to infinity problem. It is to be understood that both the aforementioned general limitations and any other limitations mentioned during correspondence are not all inclusive.


=======
Results
=======
The Program returns the initial routing table, received distance vector and its routing table after update. It should also prints out a message when it periodically sends out the distance vector.





