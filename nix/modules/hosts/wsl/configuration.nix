{ config, inputs, ... }: {

  flake.nixosConfigurations.wsl = inputs.nixpkgs.lib.nixosSystem {
    modules = [
      inputs.nixos-wsl.nixosModules.default
      config.flake.nixosModules.host-wsl
    ];
  };

  flake.nixosModules.host-wsl =
    { pkgs, lib, ... }:
    {
      imports = with config.flake.nixosModules; [
        headless-packages
        terminal-nixos
        locale
        nix
      ];

      wsl.enable = true;
      wsl.defaultUser = "ssol";

      # this line is actually load-bearing, but I don't understand why
      nixpkgs.hostPlatform = lib.mkDefault "x86_64-linux";

      networking.hostName = "wsl";

      users.extraUsers.ssol = {
        shell = pkgs.zsh;
      };

      system.stateVersion = "26.05";
    };
}
