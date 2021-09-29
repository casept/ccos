let
  sources = import ./nix/sources.nix;
  niv = import sources.niv { inherit sources; };
  pkgs = import sources.nixpkgs { };
in pkgs.mkShell {
  # So locale doesn't break on non-nixos
  LOCALE_ARCHIVE_2_27 = "${pkgs.glibcLocales}/lib/locale/locale-archive";
  nativeBuildInputs = with pkgs; [
    # Dev tooling
    bear
    bochs
    coreutils
    clang-tools
    cppcheck
    e2tools
    e2fsprogs
    flawfinder
    gnumake
    include-what-you-use
    python3
    gdb-multitarget
    (callPackage ./nix/pkgs/camomilla { pythonXXPackages = python39Packages; })
    (callPackage ./nix/pkgs/lizard { })
    (callPackage ./nix/pkgs/limine { })
    qemu

    # Nix support
    niv.niv
  ];
}
