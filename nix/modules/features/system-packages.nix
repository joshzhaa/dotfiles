_: {
  flake.nixosModules = {
    headless-packages = { pkgs, ... }: {

      environment.systemPackages = with pkgs; [
        # nix
        nixd
        statix
        nixfmt
        nh
        # cli
        git
        ripgrep
        fd
        starship
        jujutsu
        # tui
        neovim
        tree-sitter
      ];

      # overriding the most baffling default in nixos
      environment.defaultPackages = [ ];

    };

    desktop-packages = { pkgs, ... }: {

      environment.systemPackages = with pkgs; [
        obsidian
        alacritty
        chromium
      ];

    };
  };
}
