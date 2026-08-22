_: {
  perSystem = { pkgs, ... }: {
    devShells = {
      cpp = pkgs.mkShell.override { stdenv = pkgs.libcxxStdenv; } {
        packages = with pkgs; [
          clang
          clang-tools
        ];
      };
    };
  };
}
