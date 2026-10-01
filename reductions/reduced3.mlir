module attributes {dlti.dl_spec = #dlti.dl_spec<!llvm.ptr<270> = dense<32> : vector<4xi64>, !llvm.ptr<271> = dense<32> : vector<4xi64>, !llvm.ptr<272> = dense<64> : vector<4xi64>, i64 = dense<64> : vector<2xi64>, i128 = dense<128> : vector<2xi64>, f80 = dense<128> : vector<2xi64>, !llvm.ptr = dense<64> : vector<4xi64>, i1 = dense<8> : vector<2xi64>, i8 = dense<8> : vector<2xi64>, i16 = dense<16> : vector<2xi64>, i32 = dense<32> : vector<2xi64>, f16 = dense<16> : vector<2xi64>, f64 = dense<64> : vector<2xi64>, f128 = dense<128> : vector<2xi64>, "dlti.endianness" = "little", "dlti.mangling_mode" = "e", "dlti.legal_int_widths" = array<i32: 8, 16, 32, 64>, "dlti.stack_alignment" = 128 : i64>, llvm.module_asm = [], llvm.target_triple = "x86_64-redhat-linux-gnu"} {
  llvm.func @sqlite3_str_vappendf() -> i8 {
    %0 = llvm.mlir.constant(0 : i8) : i8 // %0 is 00000000
    %1 = llvm.mlir.constant(44 : i8) : i8 // %1 is 00101100
    llvm.switch %0 : i8, ^bb2(%0 : i8) [
      45: ^bb2(%0 : i8),
      44: ^bb1
    ]
  ^bb1:  // pred: ^bb0
    llvm.br ^bb2(%1 : i8)
  ^bb2(%2: i8):  // 3 preds: ^bb0, ^bb0, ^bb1
    // argument: %2 is 00T0TT00
    llvm.return %2 : i8
  }
}
