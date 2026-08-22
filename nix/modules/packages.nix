{ self, inputs, ... }: {
  perSystem = { pkgs, system, ... }: {
    packages = {
      plasma-final = pkgs.kdePackages.plasma-workspace;
      plasma-prev = inputs.nixpkgs.legacyPackages.${system}.kdePackages.plasma-workspace;

      sway-status = pkgs.callPackage ../pkgs/sway-status/package.nix { };
    };

    _module.args.pkgs = import inputs.nixpkgs {
      inherit system;
      overlays = [ self.overlays.kde ];
      config.allowUnfree = true;
    };
  };
}
