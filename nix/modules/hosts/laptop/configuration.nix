{ config, inputs, ... }: {

  flake.nixosConfigurations.laptop = inputs.nixpkgs.lib.nixosSystem {
    modules = with config.flake.nixosModules; [
      host-laptop
      host-laptop-hardware
    ];
  };

  flake.nixosModules.host-laptop =
    { pkgs, ... }:
    {
      imports = with config.flake.nixosModules; [
        headless-packages
        desktop-packages
        terminal-nixos
        locale
        sound
        sway
        nix
      ];

      # nixpkgs.overlays = [ config.flake.overlays.kde ];

      boot.loader.systemd-boot.enable = true;
      boot.loader.efi.canTouchEfiVariables = true;

      networking = {
        hostName = "laptop";
        networkmanager.enable = true;
        nameservers = [ "1.1.1.1" ];
      };

      services = {
        # Enable CUPS to print documents.
        printing.enable = true;
      };

      # Define a user account.
      users.users."ssol" = {
        isNormalUser = true;
        description = "ssol";
        extraGroups = [
          "networkmanager"
          "wheel"
        ];
        shell = pkgs.zsh;
      };

      programs.chromium.enable = true;

      fonts.packages = with pkgs.nerd-fonts; [
        jetbrains-mono
      ];

      system.stateVersion = "26.05";
    };
}
