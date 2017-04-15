# Twisted Hash

This is a very fast hashing algorithm, currently using the BLAKE2b AVX2
optimized permutation.  On my Core i7 Haswell laptop, it achieves:

    long messages: 0.66 cycles/byte
    4096 bytes: 0.77 cycles/byte
    1536 bytes: 1.62 cycles/byte
    576 bytes: 2.35 cycles/byte
    64 bytes: 6.16 cycles/byte
    8 bytes: 49.35 cycles/byte

The targetted security level is 256 bits.  It should have the equivalent
defense as 12 rounds of BLAKE2b.  For comparison, here is a fairly extensive
set of benchmarks for various algorithms:

    https://bench.cr.yp.to/results-hash.html

Samuel Neves' AVX2 optimized BLAKE2bp code is at:

    https://github.com/sneves/blake2-avx2

This implemenation combines Samuel Neves' 4-way parallel AVX2 BLAKE2b
implementation with a twisted hashing pattern to roughly double the speed.  At
the same time, it is good for both small and large messages, and efficient on
32-bit processors without SIMD units, as well as AVX2 enabled CPUs.

## Example

This idea is best explained with an example.  Suppose we have a hashing sponge
H which has a multiple of 4 hashing rounds, such Keccak-1600, which uses 24
rounds.  Assume we want to hash 8 512-bit data blocks, which I call A, B, C, D,
E, F, G, and H.  The sponge would normally absorb the sequence ABCDEFGH, and
then squeeze the result.  The total hash rounds is 24*8 = 192.

Now lets instead absorb the input stream twice, but the second time in a
different order.  The new sequence is ABCDEFGHEBGDAFCH.  The second pass simply
subtracts 3 from the previous position each time, wrapping in the natural way.

Consider the computation graph, shown below, where there are 8 nodes for the 8
message blocks, and arrows are drawn to indicate the absorb pattern.

![twisted hash graph](/TwistedHashing.png)

Look for loops in this graph.  I claim the shortest loop has 4 nodes.
We can create a faster hashing algorithm using this access pattern with equal
security.  In this case, what happens when we reduce the number of rounds from
24 to 6?  The overall hashing rounds is then 2*8*6 = 96, exactly 1/2 the number
of rounds in the original version.

What attacks can be mounted against this twisted hash?  The simplest attack
against the normal non-twisted algorithm is possibly a differential attack,
where the attacker finds a differential function for changing A into A' and B
in to B' with better chance of A' B' hashing the same as AB than when choosing
A' and B' randomly.  This is not feasible for a cryptographically strong hash,
meaning the chance per attempt in a differential attack is <=
1/(2^bit-security) of the hashing algorithm.  I assume that the chance per
attempt of finding A' B' pairs that hash the same as A B, when using 1/4 the
hashing rounds, is:

    P = 1/(2^(bit-security/4)).

In other words, I assume it is exponentially vanishing in terms of the number
of rounds.  This assumption where mathematical properties are assumed to
propate with a fixed probability per round is commonly used in differential
attacks, rotational attacks, etc.

Assume the attacker finds a differential function for modifying B given a
change to A and a current sponge state such that the likelihood that absorbing
A'B' results in the same sponge state as absorbing AB is P.  Further assume the
attacker can do the reverse, and find a change to A give a a change to B that
works with probability P.

Assume the attacker finds solutions A'B' with probability P.  We have a loop in
the computation graph:

    A->B, A->F, B->G, and F->G

The attacker only satisfied the first edge so far.  The attacker's A'B' restore
the sponge state after absorbing the sequence A'B', but later the sponge will
have to absorb B' again, which needs to be corrected by a change to G, which we
call G'.  Absorbing B'G' works with probability P, so the combined probability
so far is P^2.  Next, the attacker absorbs A', which needs to be corrected by a
change to F, called F'.  The chance of this working is now P^3.  Finally, we
failed to correct the edge F->G, which happens before absorbing B' for the
second time.  The likelihood of these somewhat unrelated changes to F and G
working out is no higher than the differential probability, so the total
combined probability is at least P^4.  Therefore, the differential attack is no
better against the 2-pass twisted hash with 1/4 the rounds than against the
original hash.

This proof can be extended to cover all attacks that assume that some
mathematicla property propagates through rounds with constant probabilty.

## Larger Graphs

This twisted absorb pattern can be extended to graphs with minimum loop sizes
of N, for arbitrary N, if the block size is large enough.  However, the block
size grows exponentially with N.  The number of hash rounds that should be
secure against a differential attack is R/N, if R is the original secure number
of rounds.  The resulting total hash rounds is 2*R/N.

In practical terms, we will want N to be not so large that the data does not
fit into L1 cache, or the second pass will load too slowly.

I found a sequence of increasing cycle-length graphs of optimal size:

min cycle 3: 5 nodes
min cycle 4: 8 nodes
min cycle 5: 17 nodes
min cycle 6: 26 nodes

These graphs are highly symmetric, and there seems to be a simple algorithm for
building the next larger one.

To find a good absorb sequence,  we need an N-cycle graph where every node has
degree 4, and where there are no cycles < N in length.  Since every vertex has
even degree, we can just find a Eulerian path, which is quick to find and
always exists, and use that as the absorb sequence.  This sequence ensures that
any given block in the sequence is absorbed twice, and at least N blocks apart,
where N is the min cycle length.  Also, if blocks A and B are adjacent in the
sequence then A and B will not appear closer than N-2 blocks apart elsewhere.

The first set of such graphs, assuming they can all be built successfully, have
these parameters:

    cycle length	4	6	8	10	12
    tree leaves	        4	12	36	108	324
    dangling edges	12	36	108	324	972
    tree nodes	        5	17	53	161	485
    new nodes	        3	9	27	81	243
    graph size	        8	26	80	242	728

The graph size is

    graphSize(2N) = 3*graphSize(2N-2) + 2.

For example, we could use 1/12 the hash rounds in 2 passes, for a 6X reduction
in computation, without sacrificing security, if we hash 728 blocks at once
(46,592 bytes for a 512-bit hash).  On the downside, cache bandwidth doubles,
which is a factor that limits how much we can speed things up.

The algorithm for building the graph is to start with a tree of depth N/2,
where each node has degree 4 (meaning the root has 4 children, and the rest
have 3).  The leaves will then be connected to each other to build odd cycle
length graphs, but for even cycle lengths, we add new nodes and connect each to
four dangling leaf edges.  The number of additional nodes is leaves*3/4, which
is always a power of 3.  The graph looks identical if you pick any one of the
new nodes and make it the root of the tree.

Look at the graph below, which is a graph of degree 4 nodes with no cycles less
than 6 long.  There are 26 nodes.  Any Eulerian path on this graph results in a
reasonable absorb sequence.  One such path is:

    abfrmypwnzkcjuqeoxltkvgbhyjsftpeacirqzhxiwguldmvosnd

![twisted hash graph](/Min6Cycle.png)
