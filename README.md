# ECCElGamal online voting

## General
Voting based on ECC ElGamal homomorphism

To build executable programs, run:

> bash build.sh

To run demo vote programs, run:
> bash demo_script.sh

## Programs structure

```md
evoting
├── config
│   ├── curve
│   ├── vote_config
├── out
│   ├── demo_count_vote.o
│   ├── demo_vote.o
│   ├── vote_prepare.o
├── plain_db
│   ├── candidates
│   ├── combination_array
│   └── combination_hash
│   ├── encrypted_votes
│   └── plain_votes
```

### Config

The two config files below must exits before preparing for the vote, must not be changed during the vote and must be configured following the format below.

* **curve**: This file contains configuration on used elliptic curve $$**E: y^2 = x^3 + ax + b (mod p)**$$. First line is **a**, second line is **b**, third line is **p**. The next two lines shall be x value and y value of the generator point later used for ECC.
* **vote_config**: This file contains configuration for online voting. First line is number of candidates, second line is number of votes.

### Executables:

* **vote_prepare.o**: Create/Update 3 files `candidates` and `combination_array`, `combination_hash` to support voting and vote counting later.
* **demo_vote.o**: Randomly vote for the candidates based on the configuration files. Show vote output on screen so that we can easily confirm the result. Create/update 2 files `encrypted_votes` and `plain_votes` to save vote result. The `plain_votes` format shall contains the number of votes for each candidate in the order of candidate number so users can check at anytime.
* **demo_count_vote.o**: Read `encrypted_votes` to get the encrypted vote result and output the number of votes for each candidate in the order of candidate number.

### Plain DB:

The data files output by the executables. DO NOT manually edit them.
