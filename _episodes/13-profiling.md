---
title: "Profiling Parallel Applications"
teaching: 30
exercises: 30
questions:
- "It works, what is taking time?"
objectives:
- "Use a profiler to find bottlenecks and hotspots and diagnose problems."
- "Reinforce the idea that profiling is an efficient way to analyse code."
keypoints:
- "Use a profiler to find the most important functions and concentrate on those."
- "The Profiler needs to understand MPI. Your cluster probably has one."
- "If a lot of time is spent in communication, maybe it can be rearranged."
---

## Profiling

Profilers help you find out where the program is spending it's time
and pinpoint places where optimising it makes sense.
In this lesson we will use Scalasca, but many other profilers exist.
Scalasca is specifically an MPI profiler, it will give you
information on communication efficiency and bottlenecks.

Scalasca is an open source application developed by the 

It is used together with the [scorep](FIXME) utility, the Cube library
and the Cube viewer.
Since it is open source, you can install it on user level on any HPC cluster 
that does not already have it.
The process is as described in [setup](setup), but you have to add 
--prefix=$HOME/scalasca/ to the ./configure command to install in
your home folder.

### Summary profile

It is advisable to create a short version of you program, limiting the
runtime to a few minutes.
You may be able to reduce the problem size or required precision,
as long as this does not change the algorithm itself.

Later we will see how we can limit the scope of the profiler, but first
we need to run a summary of the whole program.

Start by recompiling the application, replacing mpicc with the version
provided by the scorep utility.
~~~
scorep mpicc -o poisson poisson_main_mpi.c poisson_step_mpi.c
~~~
{: .output}
This will produce an instrumented binary.
When you execute, it will track function calls and the time spent in each
function.

Next, run the new executable using scalasca to analyse the result
~~~
scalasca -analyze mpirun -n 2 poisson
~~~
{: .output}

Finally, you can examine the results in the GUI
~~~
scalasca -examine scorep_poisson_2_sum
~~~
{: .output}
or on the command line
~~~
{: .output}
scalasca -examine -s scorep_poisson_2_sum
cat scorep_poisson_2_sum/scorep.score
~~~
{: .output}

The command line version will produce output that looks something like this:
~~~
Estimated aggregate size of event trace:                   1544400 bytes
Estimated requirements for largest trace buffer (max_tbc): 468100 bytes
(hint: When tracing set SCOREP_TOTAL_MEMORY > max_tbc to avoid intermediate flushes
 or reduce requirements using file listing names of USR regions to be filtered.)

flt type         max_tbc         time      % region
     ALL          468100         0.90  100.0 ALL
     MPI          428080         0.36   40.0 MPI
     COM           40020         0.54   60.0 COM

     MPI          164000         0.09    9.9 MPI_Recv
     MPI          164000         0.02    2.4 MPI_Send
     MPI          100000         0.11   12.5 MPI_Allreduce
     COM           40000         0.53   59.3 poisson_step
     COM              20         0.01    0.7 main
     MPI              20         0.14   15.2 MPI_Init
     MPI              20         0.00    0.0 MPI_Finalize
     MPI              20         0.00    0.0 MPI_Comm_size
     MPI              20         0.00    0.0 MPI_Comm_rank
~~~
{: .output}

Either way we find that in our case, the program is waiting for MPI
communication for 36% of the total runtime.
This is not completely out of the ordinary, but we could do better.
53% of the time is spent in poisson_step.
This is not counting the time spent in functions called by poisson step.
This essentially means that 53% of the time is spent in actual computation.

If you are working on a terminal and prefer not to use the GUI, Cube
can print most of the information for you
For example the following will print out the time spent in each function:
~~~
cube_stat -p scorep_poisson_2_sum/summary.cubex
~~~
{: .output}
This outputs
~~~
Routine                  time
INCL(main)           0.895395
  EXCL(main)         0.006292
  MPI_Init           0.135787
  MPI_Comm_rank      0.000008
  MPI_Comm_size      0.000004
  poisson_step       0.753207
  MPI_Finalize       0.000097
~~~
{: .output}

The INCL(main) icnludes function calls inside the main function and
EXCL(main) counts only the time spent in the main function itself.

This tells us clearly that poisson_step is the most important function
to optimise.

You can also get a call tree using
~~~
cube_calltree -m time scorep_poisson_2_sum/summary.cubex
~~~
{: .output}
will print out something like this:
~~~
Reading scorep_poisson_2_sum/summary.cubex... done.
0.00629203      main
0.135787          + MPI_Init
7.65e-06          + MPI_Comm_rank
4.06e-06          + MPI_Comm_size
0.531253          + poisson_step
0.112207          |   + MPI_Allreduce
0.0883989         |   + MPI_Recv
0.0213479         |   + MPI_Send
9.6928e-05        + MPI_Finalize
~~~
{: .output}

The numbers on the left give the percentage of time spent in each function.

> ## Options
>
> Try adding the -i flag to cube_calltree.
> What does this do?
>
>> ## Solution
>> The percentages have changed to inclusive ones.
>> They now report the whole time spent in the function,
>> including in subfunction calls.
>{: .solution}
>
{: .challenge}

>## Note on the Linux Subsystem
>
> Unfortunately the linux subsystem kernel does not support the hardware level profiling.
> If you are running of the subsystem you have significantly less information.
> In a moment we will see ways of getting more information even in this case,
> but the best option is the first two commands on an HPC cluster and examine
> the results on your personal computer.
>
{: .callout}

### Filtering

You can probably see that in a larger program the call tree would be large
and hard to manage.
The graphical interface makes this a bit easier, but the real solution is filtering.

Filtering can be used to turn off measurements in less important parts of
the code or to restrict measurements to the most interesting parts
of the code.
The filter needs to be saved as a file.
The following will filter out everything except poisson_step, effectively
restricting the measurements to this function.
~~~
SCOREP_REGION_NAMES_BEGIN
  EXCLUDE
    *
  INCLUDE
    poisson_step
SCOREP_REGION_NAMES_END
~~~
{: .output}

Save this to a file called poisson.filter.
Running 
~~~
scalasca -examine -s scorep_poisson_2_sum
cat scorep_poisson_2_sum/scorep.score
~~~
{: .output}
will produce something like
~~~
Estimated aggregate size of event trace:                   2087kB
Estimated requirements for largest trace buffer (max_buf): 637kB
Estimated memory requirements (SCOREP_TOTAL_MEMORY):       4097kB
(hint: When tracing set SCOREP_TOTAL_MEMORY=4097kB to avoid intermediate flushes
 or reduce requirements using USR regions filters.)

flt     type max_buf[B] visits time[s] time[%] time/visit[us]  region
 -       ALL    652,120 40,020    0.98   100.0          24.52  ALL
 -       MPI    604,096 32,016    0.42    43.0          13.19  MPI
 -       COM     48,024  8,004    0.56    57.0          69.84  COM

 *       ALL    652,096 40,016    0.97    98.7          24.21  ALL-FLT
 -       MPI    604,096 32,016    0.42    43.0          13.19  MPI-FLT
 *       COM     48,000  8,000    0.55    55.7          68.31  COM-FLT
 +       FLT         24      4    0.01     1.3        3136.88  FLT

 -       MPI    236,000 12,000    0.10    10.2           8.33  MPI_Recv
 -       MPI    236,000 12,000    0.02     2.3           1.92  MPI_Send
 -       MPI    132,000  8,000    0.11    11.2          13.69  MPI_Allreduce
 -       COM     48,000  8,000    0.55    55.7          68.31  poisson_step
 +       COM         24      4    0.01     1.3        3136.88  main
 -       MPI         24      4    0.19    18.9       46346.95  MPI_Init
 -       MPI         24      4    0.00     0.4        1095.21  MPI_Finalize
 -       MPI         24      4    0.00     0.0           1.08  MPI_Comm_size
 -       MPI         24      4    0.00     0.0           2.43  MPI_Comm_rank
~~~
{: .output}
The + and - sign denote that are included and filtered out.
The * character denotes a region that is partially filtered.

Once you are happy with the filter, it can then be passed to scorep with the
SCOREP_FILTERING_FILE environment variable
~~~
export SCOREP_FILTERING_FILE=poisson.filter
scorep mpicc -o poisson poisson_main_mpi.c poisson_step_mpi.c
~~~
{: .output}

And rerun the application producing the full trace
~~~
scalasca -analyze mpirun -n 2 poisson
~~~
{: .output}


## Optimisation Workflow

The general workflow for optimising a code, whether parallel or serial
is as follows
3. Profile
4. Optimise
1. Test correctness
2. Measure efficiency
5. Go to 1.



> ## Profiling
>
> Use a profiler to study your research code or the code provided.
> Identify hotspots and decide how to improve the application.
>
{: .challenge}

> ## Iterative Improvement
>
> In the Poisson code,
> try changing the location of the calls to MPI_Send. How does this affect performance?
>
{: .challenge}

{% include links.md %}
