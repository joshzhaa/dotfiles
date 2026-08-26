_: {
  perSystem = { pkgs, self', ... }: {
    devShells = {
      cpp = pkgs.mkShell.override { stdenv = pkgs.libcxxStdenv; } {
        packages = with pkgs; [
          llvmPackages.libcxxClang
          clang-tools
          lldb
        ];

        shellHook = /* bash */ ''
          export CXXFLAGS="-fprebuilt-module-path=${self'.packages.std-pcm}"
          export NIX_HARDENING_ENABLE=""  # TODO: find a way to do this less coarsely
        '';
      };
    };
  };
}
