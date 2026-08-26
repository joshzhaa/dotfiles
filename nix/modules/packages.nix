_: {
  perSystem = { pkgs, ... }: {
    packages = rec {
      # std.pcm for `import std` in c++
      std-pcm = pkgs.callPackage ../pkgs/std-pcm/package.nix { };

      statusline = pkgs.callPackage ../pkgs/statusline/package.nix { inherit std-pcm; };
    };
  };
}
