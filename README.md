# udcrgen - Generate Unit Disk Contact Embeddings for Graphs

This program will create embeddings (drawings) of caterpillar and lobster graphs, if possible.

In <q>benchmark mode</q>, the program systematically generates relevant graph instances and produces statistics on the embedding results.

## Supported Graph Classes

The algorithms implemented here pertain to lobsters and caterpillars exclusively.

Caterpillars are trees which consist of a central chain (the <q>spine</q>) and any number of leaves connected to the spine.
Lobsters are trees which consist of a <q>spine</q>, any number of <q>branch</q> vertices connected to the spine and any number of leaves connected to the branches.

For details, consult the accompanying thesis paper.

## Usage of `udcrgen`

```
udcrgen [OPTIONS...] input.graph
```

The following options are available:

* `-a`, `--algorithm` `[knp|cleve|dynamic-program|benchmark]`
* `-i`, `--input-file` `<FILE>`
* `-o`, `--output-file` `<FILE>`
* `-s`, `--stats-file` `<FILE>`
* `--archive-yes` `<DIRECTORY>`
* `--archive-no` `<DIRECTORY>`
* `-j`, `--input-format` `[degrees|edgelist]`
* `-f`, `--output-format` `[svg|ipe|dump]`
* `-e`, `--embed-order` `[depth-first|breadth-first]`
* `-g`, `--gap` `<GAP>`
* `--spine-min` `<LENGTH>`
* `--spine-max` `<LENGTH>`
* `--batch-size` `<SIZE>`
* `--benchmark-bfs` `[true|false]`
* `--benchmark-dfs` `[true|false]`
* `--benchmark-dynamic` `[true|false]`
* `-v`, `--log-level` `[silent|error|info|trace]`
* `--log-mode` `[stderr|file|both]`
* `--log-file` `<FILE>`
* `--`: end of options

For details on every command line item and synonymous options, please refer to the relevant following section.

## Algorithms

The program supports three different algorithms to run on a single input instance. The last option, benchmark mode, generates a range of instances on which two different approaches are compared.

### Klemz, Nöllenburg and Prutkin

This algorithm implements *proper* unit disk contact, i.e. two vertices are connected if and only if the disks in their output representation are in contact.
Use `-a knp`, `-a strict` or `-a strong` to choose this method.
*knp* stands for _Boris Klemz, Martin Nöllenburg, and Roman Prutkin_, authors of the paper _Recognizing weighted disk contact graphs_, which contains a description of this algorithm on caterpillars.

This program uses a configurable parameter called *gap* to visually distinguish disks which are not in contact.
Use `-g <gap>` to set the floating-point value. The default is `0.1`. The diameter of the unit disks before scaling is `1`.

The proper contact algorithm works exclusively on caterpillar graphs. Lobsters are not supported.

### Cleve

This is the default.

It implements a heuristic to realize an embedding with *weak* unit disk contact, i.e. if two vertices are connected, their two corresponding disks in the output are in contact.
Use `-a cleve` or `-a weak` to choose this method.
A description of the algorithm on caterpillars can be found in the paper by _Jonas Cleve_: _Weak Unit Disk Contact Representations for Graphs without Embedding_.

This implementation extends the approach to lobsters, while refining it with a *bend* heuristic: the spine may extend not just in a straight line, but in any of the six cardinal directions of the triangular coordinate grid.

See **Heuristic Options** for command-line options on this algorithm.

### Dynamic Program

This is a new algorithm based on a conjecture in the paper by _Sujoy Bhore, Maarten Löffler, Martin Nöllenburg and Soeren Nickel_: _Unit Disk Representations of Embedded Trees, Outerplanar and Multi-Legged Graphs_.

The conjecture is that if a weak contact embedding exists on the triangular grid for a lobster, then it admits an x-monotone embedding, i.e. an embedding in which the spine nodes are laid out with strictly increasing x coordinates.

If the conjecture holds, then it is possible to embed any tri-grid lobster in linear time by considering all the possible variations up to some specific node in order and solving the remaining partial lobster recursively. Because of x-monotonicity, only a constant number of recently blocked spaces need to be checked for collisions. The actual embedding locations of the partial solution can be disregarded.

The implemented algorithm does exactly this, thereby providing a way to potentially refute the conjecture by failing to embed some lobster which is otherwise known to have a (not necessarily x-monotone) embedding. On the other hand, if we enumerate all lobsters of certain lengths with *benchmark* mode and, for every lobster known to have a tri-grid embedding, we find an x-monotone embedding, this result is evidence to support the conjecture.

### Benchmark

With `-a benchmark`, the program runs in benchmark mode. It differs from all other algorithm settings in that there is no single input graph to be read from a file. Instead, it generates input lobsters on the fly by enumerating the possible combinations of branches on spines and leaves on branches.

To save on computation, the enumeration skips over instances which are strict extensions (with more branches or leaves) of a smaller instance of which it already knows that it cannot find an embedding. Similarly, the graphs always lean *heavy* on one end, skipping their <q>mirror graph</q> with the weight on the other end.

The program processes all generated instances with two algorithms: the `weak` contact heuristic and the `dynamic-program`.
It then records the success or failure of the respective algorithm to embed the input lobster in the statistics.
Naturally, we consider the dynamic program to be the reference algorithm, while the heuristic might not find a solution even if one exists.

The benchmark produces three types of output:

* SVG collection: successfully embedded <q>interesting</q> instances are written to HTML files
* Statistics: records on every generated input instance in CSV format
* Archive: every generated input instance in a separate file in degree-notation, ordered in directories by embedding success

An <q>interesting</q> instance in this context refers to a lobster for which the produced embedding leaves no space next to any spine or branch.
This indicates that the embedding cannot be extended to a larger instance.

See **Benchmark Options** for relevant command-line options.

## File and Directory Options

Generally, if the user does not pass some path parameter, the associated feature is disabled.

* If the `--stats-file` is not specified, there is no CSV output.
* If the `--archive-yes` or `--archive-no` directories are not specified in *benchmark* mode, the associated instances are not written out.

These exceptions apply:

* The user must specify an `--input-file`, except in *benchmark* mode.
* If the `--output-file` is not specified, the program derives the output file name from the input file name.
* If the `--output-file` is not specified in *benchmark* mode, there is simply no SVG output.

## Heuristic Options

These options affect the heuristic used for embedding the graphs.

Use `-e depth-first` to make the embedder place all leaf disks on a branch immediately after the branch disk itself.
Use `-e breadth-first` to make the embedder place all branch disks on a spine first and only then place all leaves on the branches at that spine.

Regardless of preference, the heuristic advances one spine at a time with no backtracking to previous spines and no disk can ever be embedded before its parent disk.

## Benchmark Options

In *benchmark mode*, the program generates a large number of instances by itself and applies the other algorithms to them.
The generated instances exhaustively cover all configurations for lobsters of a certain size.

Even so, not all possible lobsters are actually processed.
The generator systematically enumerates them by adding branches and leaves one by one.
When the current instance cannot be solved by any of the enabled algorithms from the reportoire, we know that strictly larger instances will also fail.
The generator thus proceeds with a smaller instance of a different shape.

Limit the scope of the instances generated for the benchmark with the `--spine-min` and `--spine-max` options.
The benchmark applies to all lobsters with a number of spine nodes greater than or equal to the minimum and smaller than the (exclusive!) maximum.
If the spine size limits are unspecified, the defaults are `2` (min) and `3` (max), a very small benchmark.

Choose the benchmarked algorithms with the following boolean options.
All algorithms implement *weak* unit disk contact and all default to `true`.

* `--benchmark-bfs`: run the heuristic algortihm with the breadth-first embed order.
* `--benchmark-dfs`: run the heuristic algortihm with the depth-first embed order.
* `--benchmark-dynamic` `[true|false]`: run the dynamic programming algortihm.

The benchmark may produce three kinds of output, all optional.

If the `--stats-file` option is specified, it will generate one statistical record per instance processed. See **Statistics** below.

If the `--output-file` option is specified, the file serves as the destination for not just one, but all solved problem instances.
Because the benchmark generates an enormous amount of instances, the output file can be limited in size. Use `--batch-size` to specify the maximum number of solutions for one SVG container file. If the batch size is unspecified or explicitly set to `0`, the program writes all SVGs to one single file.
Once the output file has reached maximum size, further solutions will be written to `file_N.html`, where `file.html` is the configured output file name and `N` is the increasing batch number.

If no output file is specified, the embedding problem changes from a construction problem to a decision problem. The dynamic program will take this into account and omit additional bookkeeping to reconstruct the solution, potentially increasing performance.

The only supported output format (`-f` parameter) in this mode is `svg`.

If the `--archive-yes` or `--archive-no` options are set to directory path(s), lobster instances which the program enumerates during the benchmark are written, one file each, into (subdirectories of) these directories.
The file format is in degree representation, one line per spine.
The archive contains the results of the dynamic programming algorithm, so it cannot be used together with `--benchmark-dynamic false`.

## Formats

The program currently has two input formats: `degrees` and `edgelist`.
Both are simple, whitespace-separated custom representations of graphs.

For `degrees`, the input file must contain a whitespace-separated list of the degrees of the spine vertices in order. The result will be a caterpillar.

For `edgelist`, the input file must contain one line for each edge, with the id of the from-vertex and the id of the to-vertex separated by a space.
ids are always integers.

The program supports SVG and IPE as output formats. Use `-f svg` and `-f ipe` respectively.

* SVG are Scalable Vector Graphics, a widespread XML-based format for vector graphics. Web browsers can display it.
* IPE is the native file format of [Ipe](https://ipe.otfried.org/), a drawing editor for creating figures in PDF format. 

Using `-f dump`, the program will output a text file generated from the internal result representation of the algorithm. It consists of lines of the form: `v -> p  (x, y)`, where

* _v_ is the number of an embedded vertex,
* _p_ is the number of the parent vertex to which _v_ connects and
* _(x, y)_ is the central coordinate of the embedded unit disk representing _v_.

## Statistics

Every time the program applies one of its algorithms to an input graph, it builds a statistical record of the result.
This record includes:

* the lobster encoded in degree notation (only in benchmark mode),
* the algorithm name (same as the `-a` command-line option),
* the embedding order (same as the `-e` command-line option),
* the total number of graph nodes in the instance,
* the number of spine nodes in the instance,
* success: `true` if the algorithm found an embedding, `false` otherwise,
* the run time of the algorithm in microseconds.

The program appends the statistical record to the CSV file specified as the `--stats-file` parameter.
One record corresponds to one CSV line.
In the case of *benchmark* mode, all enumerated problem instances are recorded in the results.

## Logging

The program implements log messages at different *levels*. It prints all messages which have a level above or at the configured *log level*, which can be set using the `--log-level` command-line option.

The available levels are, from highest to lowest:

* `silent`: no messages are logged at this level,
* `error`: conditions which directly prevent the program from operating,
* `info`: a record of key occurrences during execution, and
* `trace`: a detailed record of execution steps.

The default level is `info`.

Configure the log to write to the stderr stream, a log file, or both with the `--log-mode` option.
The default mode is `stderr`.

Configure the log file with the `--log-file` option.
The default log file is `udcrgen.log`, where `udcrgen` is the command name (<q>0th argument</q>) used to invoke the program.
If the log file is specified, then the default log mode is `file`.

For example, the following messages are logged:

| Message                             | Level |
|-------------------------------------|-------|
| (Topic) Exception: ...              | error |
| Configuration: (complete list)      | info  |
| Embed (node type) at (coordinates). | trace |

Log errors, such as failure to write to the log file, are handled by silently ignoring them, allowing the program to continue operating without log output.

## Usage of `gencases`

`gencases` is a separate, obsolete, binary included in this program.
It populates the current directory with a predefined number of input files of the following categories, where `XX` is the number of spine vertices in the input:

* `maxbranchesXX.txt`: straight-spine lobsters with a lot of branches
* `maxleavesXX.txt`: straight-spine lobsters with a lot of leaves
* `onesided_bentXX.txt`: lobsters in with every other branch is very heavy, forcing bends
* `onesided_straightXX.txt`: lobsters in with every other branch is very heavy, but still allow for a straight spine

These cases form a benchmark of sorts, but are unused because the newer *benchmark* functionality in the main program is more thorough.

## Build Instructions

This project uses the CMake build system.

It only depends on the [GoogleTest library](https://github.com/google/googletest) for unit testing.

Run the following commands from the root of the repository directory to build with optimizations for release.

```
$ mkdir build
$ cd build
$ CMAKE_BUILD_TYPE=RELEASE cmake ..
$ cmake --build .
```
