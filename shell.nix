let
  sources = import ./nix/sources.nix;
  niv = import sources.niv { inherit sources; };
  pkgs = import sources.nixpkgs {};
in
pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    # Dev tooling
    bear
    coreutils
    clang-tools
    cppcheck
    flawfinder
    gnumake
    include-what-you-use
    python3
    gdb-multitarget
    (callPackage ./nix/pkgs/camomilla { pythonXXPackages = python39Packages; })
    (callPackage ./nix/pkgs/lizard {})
    qemu

    # Nix support
    niv.niv
  ];
}
