{ moduleWithSystem, ... }: {
  flake.nixosModules.sway = moduleWithSystem (
    { self', ... }: { pkgs, ... }: {
      # Sway WM
      programs.sway = {
        enable = true;
        wrapperFeatures.gtk = true;
      };

      # nixos wiki has these in its minimal config
      environment.systemPackages = with pkgs; [
        # TODO: is this necessary?
        # wl-clipboard # clipboard, obviously
        mako # notifications
        # self'.packages.statusline # TODO : fix the packaging here
      ];

      services = {
        gnome.gnome-keyring.enable = true;
        system76-scheduler.enable = true;
        pipewire.enable = true;
      };
    }
  );
}
