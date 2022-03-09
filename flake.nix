{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
    nix-direnv.url = "github:nix-community/nix-direnv";
  };

  outputs = { self, nixpkgs, flake-utils, nix-direnv }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        devShell = pkgs.mkShell {
          buildInputs = with pkgs; [
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
            gdb
            (callPackage ./nix/pkgs/camomilla { pythonXXPackages = python310Packages; })
            (callPackage ./nix/pkgs/lizard { })
            (callPackage ./nix/pkgs/limine { })
            qemu
          ];
        };
      });
}
