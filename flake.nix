{
  description = "Glace is a GObject library to manage Wayland clients and retrieve information about them";

  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";

  outputs = {
    self,
    nixpkgs,
  }: let
    version = "0.1";
    system = "x86_64-linux";
    pkgs = import nixpkgs {inherit system;};

    nativeBuildInputs = with pkgs; [
      gobject-introspection
      meson
      pkg-config
      ninja
      vala
    ];

    buildInputs = with pkgs; [
      glib
      gtk3
    ];
  in {
    packages.${system} = rec {
      default = glace;
      glace = pkgs.stdenv.mkDerivation {
        inherit nativeBuildInputs buildInputs;
        pname = "libglace";
        version = version;
        src = ./.;
        outputs = ["out" "dev"];
      };
    };

    devShells.${system} = {
      default = pkgs.mkShell {
        inherit nativeBuildInputs buildInputs;
      };
    };
  };
}
