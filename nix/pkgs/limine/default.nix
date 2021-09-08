{ stdenv, lib, fetchurl }:

# No build from source for now, because the Makefile does gross stuff like cloning git repos
stdenv.mkDerivation rec {
  pname = "limine-bin";
  version = "2.52";

  src = fetchurl {
    url =
      "https://github.com/limine-bootloader/limine/raw/v2.52-binary/limine-install-linux-x86_64";
    hash = "sha256-MFHN2IydlD4F8xRMt7/Ew/COlAebDe6Jy3wYM5JR2B4=";
    executable = true;
    name = "limine-install";
  };

  phases = [ "installPhase" ];

  installPhase = ''
    mkdir -p $out/bin
    cp $src $out/bin/limine-install
  '';
}
