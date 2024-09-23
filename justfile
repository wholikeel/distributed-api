
entry := "geocli"
cmakeout := "./out"



build:
    cmake -S . -B {{cmakeout}}
    cmake --build {{cmakeout}}

run:
    ./out/src/{{entry}}

