{ python3Packages }:
with python3Packages;
buildPythonApplication {
  pname = "http-p2p-proxy";
  version = "0.1";

  propagatedBuildInputs = [ flask ];

  src = ./.;
}

