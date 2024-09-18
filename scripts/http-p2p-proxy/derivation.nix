{ python312Packages }:
with python312Packages;
buildPythonApplication {
  pname = "http-p2p-proxy";
  version = "0.1";

  propagatedBuildInputs = [
    flask
    flask-socketio
  ];

  src = ./.;
}
