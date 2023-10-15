{
  description = "JSON Serializer C for Embedded project";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
  }:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = import nixpkgs {inherit system;};
    in {
      devShell = pkgs.mkShell {
        packages = with pkgs; [
          gcc13          

          # build system
          meson
          ninja
          pkg-config
          cmake

          # tests
          cmocka
        ];  
      };
      
      formatter = pkgs.alejandra;
    });
}
