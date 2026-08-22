_: {
  flake.nixosModules.sway = { pkgs, ... }: {
    # Sway WM
    programs.sway = {
      enable = true;
      wrapperFeatures.gtk = true;
    };

    # nixos wiki has these in its minimal config
    environment.systemPackages = with pkgs; [
      wl-clipboard # clipboard, obviously
      mako # notifications
    ];

    services = {
      gnome.gnome-keyring.enable = true;
      system76-scheduler.enable = true;
      pipewire.enable = true;
    };
  };
}
