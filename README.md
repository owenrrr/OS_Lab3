## Experiment Goal

The focus of this experiment is: becoming familiar with the FAT12 file system, gcc+nasm joint compilation, and understanding the basic contents of real mode and protected mode.
- FAT12 image viewing tool
  Use C/C++ and nasm to write a FAT12 image viewing tool, read an .img format file and respond to user input



### Function list

1. After running the program, read the FAT12 image file and prompt the user to enter instructions
2. The user enters the ls path and the file and directory list of the root directory and its subdirectories is output.
1. First output the path name, add a colon:, break the line, and then output the file and directory list;
2. Use red (\033[31m) color to output the file name of the directory, and do not add a special color to the file name of the output file.
3. When the user executes the ls command without adding any options, each file/directory entry is separated by two spaces.
4. When the user adds -l as a parameter,
1. After the path name and before the colon, output the number of direct subdirectories and direct subfiles in this directory. Use "
Space connection. These two numbers do not add special colors
2. Each file/directory occupies one line. After outputting the file/directory name, leave a space, and then:
1. If the item is a directory, output the number of direct subdirectories and direct subfiles in this directory, with a space between the two numbers.
connect. These two numbers do not add special colors
1. Do not output the number of subdirectories and subfiles of the . and .. directories.
2. If the item is a file, output the size of the file
3. For the -l parameter, the user can set the -l parameter any number of times at any position in the command, but can only set the file name once.
4. Direct subdirectories are not counted. and..
5. When the user gives unsupported command parameters, an error is reported
6. When the user does not set a path, the default path is the root directory of the image file.
3. The user inputs the cat file name, and the output path corresponds to the file content. If the path does not exist or is not an ordinary file, a prompt will be given.
The content of the display is not strictly limited, but it must reflect the error.
4. The user enters exit to exit the program.



### Done

- Implement basic functions (ls, ls -l, cat)
- The cat command supports outputting files exceeding 512 bytes(Extra point)