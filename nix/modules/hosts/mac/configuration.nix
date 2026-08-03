{ config, inputs, ... }: {
  flake.darwinConfigurations.mac = inputs.nix-darwin.lib.darwinSystem {
    modules = [ config.flake.darwinModules.host-mac ];
  };

  flake.darwinModules.host-mac = { pkgs, ... }: {
    nix.enable = false; # opt out of nix-darwin managed nix installation

    fonts.packages = [ pkgs.nerd-fonts.hack ];
  };
}
