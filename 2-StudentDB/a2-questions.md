## Assignment 2 Questions

#### Directions
Please answer the following questions and submit in your repo for the second assignment.  Please keep the answers as short and concise as possible.

1. In this assignment I asked you provide an implementation for the `get_student(...)` function because I think it improves the overall design of the database application.   After you implemented your solution do you agree that externalizing `get_student(...)` into it's own function is a good design strategy?  Briefly describe why or why not.

    > **Answer**:  _I think that it is a good strategy, as its functionality ended up being used in several other functions. Whenever you might be performing the same operation in several different places, and especially one that is fairly complex, it is best to write a seperate function for that operation._

2. Another interesting aspect of the `get_student(...)` function is how its function prototype requires the caller to provide the storage for the `student_t` structure:

    ```c
    int get_student(int fd, int id, student_t *s);
    ```

    Notice that the last parameter is a pointer to storage **provided by the caller** to be used by this function to populate information about the desired student that is queried from the database file. This is a common convention (called pass-by-reference) in the `C` programming language. 

    In other programming languages an approach like the one shown below would be more idiomatic for creating a function like `get_student()` (specifically the storage is provided by the `get_student(...)` function itself):

    ```c
    //Lookup student from the database
    // IF FOUND: return pointer to student data
    // IF NOT FOUND: return NULL
    student_t *get_student(int fd, int id){
        student_t student;
        bool student_found = false;
        
        //code that looks for the student and if
        //found populates the student structure
        //The found_student variable will be set
        //to true if the student is in the database
        //or false otherwise.

        if (student_found)
            return &student;
        else
            return NULL;
    }
    ```
    Can you think of any reason why the above implementation would be a **very bad idea** using the C programming language?  Specifically, address why the above code introduces a subtle bug that could be hard to identify at runtime? 

    > **ANSWER:** _The reason this code would produce a bug is that the student_t struct that is being returned is allocated on the *stack* instead of the *heap*. Once a function finishes running, any stack memory allocated within the function is made available again automatically, with the variables referencing that memory being deleted. I would imagine this would have one of two effects, if not both: first, the program might write over the student variable's address as it is viewed as available; second, accessing that address later in the program could cause a segmentation fault._

3. Another way the `get_student(...)` function could be implemented is as follows:

    ```c
    //Lookup student from the database
    // IF FOUND: return pointer to student data
    // IF NOT FOUND or memory allocation error: return NULL
    student_t *get_student(int fd, int id){
        student_t *pstudent;
        bool student_found = false;

        pstudent = malloc(sizeof(student_t));
        if (pstudent == NULL)
            return NULL;
        
        //code that looks for the student and if
        //found populates the student structure
        //The found_student variable will be set
        //to true if the student is in the database
        //or false otherwise.

        if (student_found){
            return pstudent;
        }
        else {
            free(pstudent);
            return NULL;
        }
    }
    ```
    In this implementation the storage for the student record is allocated on the heap using `malloc()` and passed back to the caller when the function returns. What do you think about this alternative implementation of `get_student(...)`?  Address in your answer why it work work, but also think about any potential problems it could cause.  
    
    > **ANSWER:** _This implementation is a lot better and will not run into the same issues as the code in question 2. Memory allocated on the heap is not freed until the programmer decides to free it, so we wouldn't need to worry about our data being overwritten or a seg fault happening. However, if the programmer is not careful, they could forget to free the memory returned by this function, leading to memory leaks._  


4. Lets take a look at how storage is managed for our simple database. Recall that all student records are stored on disk using the layout of the `student_t` structure (which has a size of 64 bytes).  Lets start with a fresh database by deleting the `student.db` file using the command `rm ./student.db`.  Now that we have an empty database lets add a few students and see what is happening under the covers.  Consider the following sequence of commands:

    ```bash
    > ./sdbsc -a 1 john doe 345
    > ls -l ./student.db
        -rw-r----- 1 bsm23 bsm23 128 Jan 17 10:01 ./student.db
    > du -h ./student.db
        4.0K    ./student.db
    > ./sdbsc -a 3 jane doe 390
    > ls -l ./student.db
        -rw-r----- 1 bsm23 bsm23 256 Jan 17 10:02 ./student.db
    > du -h ./student.db
        4.0K    ./student.db
    > ./sdbsc -a 63 jim doe 285 
    > du -h ./student.db
        4.0K    ./student.db
    > ./sdbsc -a 64 janet doe 310
    > du -h ./student.db
        8.0K    ./student.db
    > ls -l ./student.db
        -rw-r----- 1 bsm23 bsm23 4160 Jan 17 10:03 ./student.db
    ```

    For this question I am asking you to perform some online research to investigate why there is a difference between the size of the file reported by the `ls` command and the actual storage used on the disk reported by the `du` command.  Understanding why this happens by design is important since all good systems programmers need to understand things like how linux creates sparse files, and how linux physically stores data on disk using fixed block sizes.  Some good google searches to get you started: _"lseek syscall holes and sparse files"_, and _"linux file system blocks"_.  After you do some research please answer the following:

    - Please explain why the file size reported by the `ls` command was 128 bytes after adding student with ID=1, 256 after adding student with ID=3, and 4160 after adding the student with ID=64? 

        > **ANSWER:** _the ls command reports those sizes because the EOF signal is located after those bytes. A record with an ID of 1 is stored at byte 64, and with a length of 64 it spans to byte 127 (128 would then be the EOF). Similarly, a record with ID of 3 is stored at byte 192, and spans until byte 255. Finally, a record with ID of 64 is stored at byte 4096 and spans until byte 4159. In each case, there is no other data at the end of the file (when the ls command is run), so the byte after will be the number of bytes reported by the ls command as the file size._

    -   Why did the total storage used on the disk remain unchanged when we added the student with ID=1, ID=3, and ID=63, but increased from 4K to 8K when we added the student with ID=64? 

        > **ANSWER:** _Despite what the ls command said, Linux stores files on the disk in blocks of set byte size, the smallest being 4KB, or, more precisely, 4096 bytes; even though the file may contain less data, as was the case when ID=1 and ID=3 was added, the file still takes up 4KB on the disk. The ID of 63 creates a record putting the end of the file just at that 4096 byte mark, so the file will still take up 4KB of disk space. Hoever, when the record with ID=64 is added, another block of memory is needed, and despite there only being 64 bytes of additional memory, the disk will allocate another block, bringing the total storage used up to 8KB._

    - Now lets add one more student with a large student ID number  and see what happens:

        ```bash
        > ./sdbsc -a 99999 big dude 205 
        > ls -l ./student.db
        -rw-r----- 1 bsm23 bsm23 6400000 Jan 17 10:28 ./student.db
        > du -h ./student.db
        12K     ./student.db
        ```
        We see from above adding a student with a very large student ID (ID=99999) increased the file size to 6400000 as shown by `ls` but the raw storage only increased to 12K as reported by `du`.  Can provide some insight into why this happened?

        > **ANSWER:**  _The ls command is reporting that file size because the last data in that file are that big record, spanning from bytes 6,399,936 to byte 6,399,999 (then, byte 6,400,000 being the EOF). However, if a file contains holes, those holes are not provided any disk space (that is, if the hole spans an entire block). This file was using 2 blocks before, and after adding this record it is using an additional block separated from the rest by a hole; hence, the total disk usage for this file will be 3*4 = 12KB._
