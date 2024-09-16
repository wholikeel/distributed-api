{ stdenv
, cmake
, gtest
}:
stdenv.mkDerivation {
    pname = "code-jame-dapi";
    version = "0.0.1";
    src = ./.;
    nativeBuildInputs = [ cmake ];
    buildInputs = [ gtest ];
}


