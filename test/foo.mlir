module attributes {dlti.dl_spec = #dlti.dl_spec<!llvm.ptr<270> = dense<32> : vector<4xi64>, !llvm.ptr<271> = dense<32> : vector<4xi64>, !llvm.ptr<272> = dense<64> : vector<4xi64>, i64 = dense<64> : vector<2xi64>, i128 = dense<128> : vector<2xi64>, f80 = dense<128> : vector<2xi64>, !llvm.ptr = dense<64> : vector<4xi64>, i1 = dense<8> : vector<2xi64>, i8 = dense<8> : vector<2xi64>, i16 = dense<16> : vector<2xi64>, i32 = dense<32> : vector<2xi64>, f16 = dense<16> : vector<2xi64>, f64 = dense<64> : vector<2xi64>, f128 = dense<128> : vector<2xi64>, "dlti.endianness" = "little", "dlti.mangling_mode" = "e", "dlti.legal_int_widths" = array<i32: 8, 16, 32, 64>, "dlti.stack_alignment" = 128 : i64>, llvm.ident = "clang version 22.1.8 (Fedora 22.1.8-4.fc44)", llvm.module_asm = [], llvm.target_triple = "x86_64-redhat-linux-gnu"} {
  llvm.module_flags [#llvm.mlir.module_flag<error, "wchar_size", 4 : i32>, #llvm.mlir.module_flag<max, "uwtable", 2 : i32>]
  llvm.func local_unnamed_addr @foo(%arg0: i8 {llvm.noundef, llvm.zeroext}, %arg1: i8 {llvm.noundef, llvm.zeroext}) -> (i8 {llvm.noundef, llvm.zeroext}) attributes {dso_local, memory_effects = #llvm.memory_effects<other = none, argMem = none, inaccessibleMem = none, errnoMem = none, targetMem0 = none, targetMem1 = none>, no_unwind, passthrough = ["mustprogress", "nofree", "norecurse", "nosync", ["min-legal-vector-width", "0"], ["no-trapping-math", "true"], ["stack-protector-buffer-size", "8"], ["target-cpu", "x86-64"]], target_cpu = "x86-64", target_features = #llvm.target_features<["+cmov", "+cx8", "+fxsr", "+mmx", "+sse", "+sse2", "+x87"]>, tune_cpu = "generic", uwtable_kind = #llvm.uwtableKind<async>, will_return} {
    %0 = llvm.mlir.constant(5 : i8) : i8
    %1 = llvm.mlir.constant(10 : i8) : i8
    %2 = llvm.add %0, %1 : i8
    %3 = llvm.xor %0, %1 : i8
    %4 = llvm.sub %0, %1 : i8
    %5 = llvm.and %0, %1 : i8
    %6 = llvm.or  %0, %1 : i8
    %7 = llvm.add %6, %arg1 : i8
    llvm.return %7 : i8
  }
}
