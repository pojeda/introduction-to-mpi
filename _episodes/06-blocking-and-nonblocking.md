---
title: "Non-blocking Communication"
teaching: 10
exercises: 10
questions:
- "Can I start the transfer and continue the program?"
objectives:
- "Introduce MPI_Isend, MPI_Irecv, MPI_Test and MPI_Wait"
keypoints:
- "Non-blocking functions allows interleaving communication and computation"
---

## Non-Blocking Communication

In one of the previous lessons we used the MPI_Send and MPI_Recv functions
to communicate between the ranks.
We saw that these functions are blocking, they will only return once the program
can safely modify the buffer.
This is safe and usually straightforward,
but we also saw how this can lead to problems.

The MPI standard includes non-blocking versions of the send and receive functions,
MPI_ISend and MPI_IRecv.
These function will return immediately, giving you more control of the flow
of the program. After calling them, it is not safe to modify the sending or
the receiving buffer, but the program is free to continue with other operations.
When it needs the data in the buffers, it needs to make sure the process is complete
using the MPI_Wait function.

> ## MPI_Isend and MPI_Irecv in C
>
>~~~
> MPI_Isend(
>    void* data,
>    int count,
>    MPI_Datatype datatype,
>    int destination,
>    int tag,
>    MPI_Comm communicator,
>    MPI_Request* request)
>~~~
>
> | data:         | Pointer to the start of the data being sent |
> | count:        | Number of elements to send |
> | datatype:     | The type of the data being sent |
> | destination:  | The rank number of the rank the data will be sent to |
> | tag:          | A message tag (integer) |
> | communicator: | The communicator (we have used MPI_COMM_WORLD in earlier) |
> | request:      | Pointer for writing the request structure |
>
>~~~
> MPI_Irecv(
>    void* data,
>    int count,
>    MPI_Datatype datatype,
>    int source,
>    int tag,
>    MPI_Comm communicator,
>    MPI_Request* request)
>~~~
>
> | data:         | Pointer to where the received data should be written |
> | count:        | Maximum number of elements received |
> | datatype:     | The type of the data being received |
> | source:       | The rank number of the rank sending the data |
> | tag:          | A message tag (integer) |
> | communicator: | The communicator (we have used MPI_COMM_WORLD in earlier) |
> | request:      | Pointer for writing the request structure |
>
{: .prereq .foldable}

> ## MPI_Isend and MPI_Irecv in Fortran
>
>~~~
> MPI_SEND(BUF, COUNT, DATATYPE, DEST, TAG, COMM, REQUEST, IERROR)
>    <type>    BUF(*)
>    INTEGER    COUNT, DATATYPE, DEST, TAG, COMM, REQUEST, IERROR
>~~~
>
> | BUF:      | Vector containing the data to send |
> | COUNT:    | Number of elements to send |
> | DATATYPE: | The type of the data being sent |
> | DEST:     | The rank number of the rank the data will be sent to |
> | TAG:      | A message tag (integer) |
> | COMM:     | The communicator (we have used MPI_COMM_WORLD in earlier) |
> | REQUEST:  | Request handle |
> | IERROR:   | Error status |
>
>~~~
> MPI_RECV(BUF, COUNT, DATATYPE, SOURCE, TAG, COMM, STATUS, REQUEST, IERROR)
>    <type>    BUF(*)
>    INTEGER    COUNT, DATATYPE, SOURCE, TAG, COMM,
>    INTEGER    STATUS(MPI_STATUS_SIZE), REQUEST, IERROR
>~~~
>
> | BUF:      | Vector the received data should be written to             |
> | COUNT:    | Maximum number of elements received                       |
> | DATATYPE: | The type of the data being received                       |
> | SOURCE:   | The rank number of the rank sending the data              |
> | TAG:      | A message tag (integer)                                   |
> | COMM:     | The communicator (we have used MPI_COMM_WORLD in earlier) |
> | REQUEST:  | Request handle                                            |
> | IERROR:   | Error status |
>
{: .prereq .foldable}

There's one new parameter here, the request.
This is used to keep track of each separate transfer started by the program.
You can check the status of a transfer using the MPI_Test function,
or call MPI_Wait to wait until the transfer is complete.

> ## MPI_Test and MPI_Wait in C
>
>~~~
> MPI_Test(
>    MPI_Request* request,
>    int * flag,
>    MPI_Status* status)
>~~~
>
> | request:      | The request |
> | flag:         | Pointer for writing the result of the test |
> | status:       | A pointer for writing the exit status of the MPI command |
>
>~~~
> MPI_Wait(
>    MPI_Request* request,
>    MPI_Status* status)
>~~~
>
> | request:      | The request |
> | status:       | A pointer for writing the exit status of the MPI command |
>
{: .prereq .foldable}

> ## MPI_Test and MPI_Wait in Fortran
>
>~~~
> MPI_TEST(REQUEST, FLAG, STATUS, IERROR)
>    LOGICAL    FLAG
>    INTEGER    REQUEST, STATUS(MPI_STATUS_SIZE), IERROR
>~~~
>
> | REQUEST:  | The request |
> | FLAG:     | Pointer for writing the result of the test |
> | STATUS:   | A pointer for writing the exit status of the MPI command |
> | IERROR:   | Error status |
>
>~~~
>MPI_WAIT(REQUEST, STATUS, IERROR)
>    INTEGER    REQUEST, STATUS(MPI_STATUS_SIZE), IERROR
>~~~
>
> | REQUEST:  | The request |
> | STATUS:   | A pointer for writing the exit status of the MPI command |
> | IERROR:   | Error status |
>
{: .prereq .foldable}



> ## Non-Blocking Communication
>
> Here is the blocking example again.
> Fix the problem using MPI_Isend, MPI_Irecv and MPI_Wait.
>
> > ## C
> > ~~~
> > #include <stdio.h>
> > #include <mpi.h>
> > 
> > int main(int argc, char** argv) {
> >    int rank, n_ranks, neighbour;
> >    int n_numbers = 1048576;
> >    int send_message[n_numbers];
> >    int recv_message[n_numbers];
> >    MPI_Status status;
> > 
> >    // Firt call MPI_Init
> >    MPI_Init(&argc, &argv);
> > 
> >    // Get my rank and the number of ranks
> >    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
> >    MPI_Comm_size(MPI_COMM_WORLD, &n_ranks);
> > 
> >    // Call the other rank the neighbour
> >    if( rank == 0 ){
> >       neighbour = 1;      
> >    } else {
> >       neighbour = 0;
> >    }
> > 
> >    // Generate numbers to send
> >    for( int i=0; i<n_numbers; i++){
> >       send_message[i] = i;
> >    }
> > 
> >    // Send the message to other rank
> >    MPI_Send(send_message, n_numbers, MPI_INT, neighbour, 0, MPI_COMM_WORLD);
> > 
> >    // Receive the message from the other rank
> >    MPI_Recv(recv_message, n_numbers, MPI_INT, neighbour, 0, MPI_COMM_WORLD, &status);
> >    printf("Message received by rank %d \n", rank);
> > 
> >    // Call finalize at the end
> >    MPI_Finalize();
> > }
> > ~~~
> > {: .output}
> {: .prereq .foldable}
>
>
>
> > ## Solution in C
> > 
> > ~~~
> > #include <stdio.h>
> > #include <mpi.h>
> > 
> > int main(int argc, char** argv) {
> >    int rank, n_ranks, neighbour;
> >    int n_numbers = 1048576;
> >    int send_message[n_numbers];
> >    int recv_message[n_numbers];
> >    MPI_Status status;
> >    MPI_Request* request;
> > 
> >    // Firt call MPI_Init
> >    MPI_Init(&argc, &argv);
> > 
> >    // Get my rank and the number of ranks
> >    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
> >    MPI_Comm_size(MPI_COMM_WORLD, &n_ranks);
> >
> >    // Call the other rank the neighbour
> >    if( rank == 0 ){
> >       neighbour = 1;      
> >    } else {
> >       neighbour = 0;
> >    }
> > 
> >    // Generate numbers to send
> >    for( int i=0; i<n_numbers; i++){
> >       send_message[i] = i;
> >    }
> > 
> >    // Send the message to other rank
> >    MPI_ISend(send_message, n_numbers, MPI_INT, neighbour, 0, MPI_COMM_WORLD, &request);
> > 
> >    // Receive the message from the other rank
> >    MPI_IRecv(recv_message, n_numbers, MPI_INT, neighbour, 0, MPI_COMM_WORLD, &request);
> >    MPI_Wait( &request, &status )
> >    printf("Message received by rank %d \n", rank);
> > 
> >    // Call finalize at the end
> >    MPI_Finalize();
> > }
> > ~~~
> > {: .output}
> {: .solution}
>
>
{: .challenge}




{% include links.md %}
