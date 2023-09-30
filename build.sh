if ! [ -e "out" ]; then
	mkdir out
fi

# g++ -I. test.cpp miracllib/poly.o miracllib/polymod.o miracllib/big.o miracllib/ecn.o miracllib/crt.o miracllib/miracl.a /usr/lib/x86_64-linux-gnu/libm.a -o out/test
# g++ -I. multiply.cpp miracllib/poly.o miracllib/polymod.o miracllib/big.o miracllib/ecn.o miracllib/crt.o miracllib/miracl.a /usr/lib/x86_64-linux-gnu/libm.a -o out/multiply.o
# g++ -I. plus.cpp miracllib/poly.o miracllib/polymod.o miracllib/big.o miracllib/ecn.o miracllib/crt.o miracllib/miracl.a /usr/lib/x86_64-linux-gnu/libm.a -o out/plus.o
g++ -I. vote_prepare.cpp miracllib/poly.o miracllib/polymod.o miracllib/big.o miracllib/ecn.o miracllib/crt.o miracllib/miracl.a /usr/lib/x86_64-linux-gnu/libm.a -o out/vote_prepare.o
g++ -I. demo_vote.cpp miracllib/poly.o miracllib/polymod.o miracllib/big.o miracllib/ecn.o miracllib/crt.o miracllib/miracl.a /usr/lib/x86_64-linux-gnu/libm.a -o out/demo_vote.o
g++ -I. vote_cmd.cpp miracllib/poly.o miracllib/polymod.o miracllib/big.o miracllib/ecn.o miracllib/crt.o miracllib/miracl.a /usr/lib/x86_64-linux-gnu/libm.a -o out/vote_cmd.o
g++ -I. demo_count_vote.cpp miracllib/poly.o miracllib/polymod.o miracllib/big.o miracllib/ecn.o miracllib/crt.o miracllib/miracl.a /usr/lib/x86_64-linux-gnu/libm.a -o out/demo_count_vote.o