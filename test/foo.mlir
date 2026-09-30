module attributes {dlti.dl_spec = #dlti.dl_spec<!llvm.ptr<270> = dense<32> : vector<4xi64>, !llvm.ptr<271> = dense<32> : vector<4xi64>, !llvm.ptr<272> = dense<64> : vector<4xi64>, i64 = dense<64> : vector<2xi64>, i128 = dense<128> : vector<2xi64>, f80 = dense<128> : vector<2xi64>, !llvm.ptr = dense<64> : vector<4xi64>, i1 = dense<8> : vector<2xi64>, i8 = dense<8> : vector<2xi64>, i16 = dense<16> : vector<2xi64>, i32 = dense<32> : vector<2xi64>, f16 = dense<16> : vector<2xi64>, f64 = dense<64> : vector<2xi64>, f128 = dense<128> : vector<2xi64>, "dlti.endianness" = "little", "dlti.mangling_mode" = "e", "dlti.legal_int_widths" = array<i32: 8, 16, 32, 64>, "dlti.stack_alignment" = 128 : i64>, llvm.ident = "clang version 22.1.8 (Fedora 22.1.8-4.fc44)", llvm.module_asm = [], llvm.target_triple = "x86_64-redhat-linux-gnu"} {
  llvm.module_flags [#llvm.mlir.module_flag<error, "wchar_size", 4 : i32>, #llvm.mlir.module_flag<max, "uwtable", 2 : i32>, #llvm.mlir.module_flag<max, "frame-pointer", 2 : i32>]
  llvm.func @foo(%arg0: i8 {llvm.noundef, llvm.zeroext}, %arg1: i8 {llvm.noundef, llvm.zeroext}) -> (i8 {llvm.zeroext}) attributes {dso_local, frame_pointer = #llvm.framePointerKind<all>, no_inline, no_unwind, optimize_none, passthrough = [["min-legal-vector-width", "0"], ["no-trapping-math", "true"], ["stack-protector-buffer-size", "8"], ["target-cpu", "x86-64"]], target_cpu = "x86-64", target_features = #llvm.target_features<["+cmov", "+cx8", "+fxsr", "+mmx", "+sse", "+sse2", "+x87"]>, tune_cpu = "generic", uwtable_kind = #llvm.uwtableKind<async>} {
    %0 = llvm.mlir.constant(1 : i32) : i32
    %1 = llvm.mlir.constant(0 : i8) : i8
    %2 = llvm.mlir.constant(-88 : i8) : i8
    %3 = llvm.alloca %0 x i8 {alignment = 1 : i64} : (i32) -> !llvm.ptr
    %4 = llvm.alloca %0 x i8 {alignment = 1 : i64} : (i32) -> !llvm.ptr
    %5 = llvm.alloca %0 x i8 {alignment = 1 : i64} : (i32) -> !llvm.ptr
    %6 = llvm.alloca %0 x i8 {alignment = 1 : i64} : (i32) -> !llvm.ptr
    llvm.store %arg0, %3 {alignment = 1 : i64} : i8, !llvm.ptr
    llvm.store %arg1, %4 {alignment = 1 : i64} : i8, !llvm.ptr
    llvm.store %1, %5 {alignment = 1 : i64} : i8, !llvm.ptr
    llvm.store %2, %6 {alignment = 1 : i64} : i8, !llvm.ptr
    %7 = llvm.load %3 {alignment = 1 : i64} : !llvm.ptr -> i8
    %8 = llvm.zext %7 : i8 to i32
    %9 = llvm.load %4 {alignment = 1 : i64} : !llvm.ptr -> i8
    %10 = llvm.zext %9 : i8 to i32
    %11 = llvm.add %8, %10 overflow<nsw> : i32
    %12 = llvm.load %5 {alignment = 1 : i64} : !llvm.ptr -> i8
    %13 = llvm.zext %12 : i8 to i32
    %14 = llvm.add %11, %13 overflow<nsw> : i32
    %15 = llvm.load %6 {alignment = 1 : i64} : !llvm.ptr -> i8
    %16 = llvm.zext %15 : i8 to i32
    %17 = llvm.add %14, %16 overflow<nsw> : i32
    %18 = llvm.trunc %17 : i32 to i8
    llvm.return %18 : i8
  }
}
