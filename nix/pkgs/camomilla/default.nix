{ fetchFromGitHub, pythonXXPackages }:
pythonXXPackages.buildPythonPackage rec {
  pname = "camomilla";
  version = "aebe89ed3b077bca4bb06562957f34addc471cd5";
  src = fetchFromGitHub {
    owner = "SuperV1234";
    repo = "camomilla";
    rev = "aebe89ed3b077bca4bb06562957f34addc471cd5";
    sha256 = "sha256-AAo0YdumQLstnPqi1Kd1mZaUKw4Zdsm+hw7SnWQFbv4=";
  };
}
