{
  writers,
  llvmPackages,
  libcxx,
  std-pcm,
}:
let
  writeCXX = writers.makeBinWriter {
    compileScript = /* bash */ ''
      NIX_HARDENING_ENABLE="" ${llvmPackages.libcxxClang}/bin/clang++ \
        -std=c++26 -fmodule-file=std=${std-pcm}/std.pcm -L${libcxx}/lib -O3 \
        $contentPath -o $out
    '';
  };
in
writeCXX "/bin/statusline" ./statusline.cc
