This is simple tool to replace binary blocks in a file.

Example :
 hexreplace file1 file2 DEADC0DE 01020304

It reads whole file1 in RAM, replaces binary then writes to file2.
So its not suitable for very large files.
