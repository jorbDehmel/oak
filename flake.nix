{
  inputs.systems.url = "github:nix-systems/default";

  outputs = {
    self,
    nixpkgs,
    flake-utils,
    systems,
  }:
    flake-utils.lib.eachSystem (import systems)
    (system: let
      pkgs = import nixpkgs {
        inherit system;
      };
      buildInputs = with pkgs; [
        clang
      ];
    in {
      packages.default = pkgs.stdenv.mkDerivation {
        name="CoolName";
        src=./.;
        inherit system;
        inherit buildInputs;
      };

      devShells.default = pkgs.mkShell {
        inherit buildInputs;
      };
    });
}

