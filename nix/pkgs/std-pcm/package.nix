{
  bash,
  coreutils,
  lib,
  libcxx,
  libcxxStdenv,
}:
derivation {
  name = "std-pcm";
  builder = "${bash}/bin/bash";
  args = [
    "-c"
    /* bash */ ''
      mkdir -p $out
      clang++ -std=c++26 --precompile -o $out/std.pcm ${libcxx}/share/libc++/v1/std.cppm
    ''
  ];
  # nix hardening gives printf and other libc functions internal linkage, which is incompatible
  # with std.cppm, which attempts to reexport those symbols from libc.
  NIX_HARDENING_ENABLE = "";
  PATH = lib.makeBinPath [
    coreutils
    libcxxStdenv.cc
  ];

  inherit (libcxxStdenv) system;
}
