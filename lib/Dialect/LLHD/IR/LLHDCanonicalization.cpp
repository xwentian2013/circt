//===- LLHDCanonicalization.cpp - Register LLHD Canonicalization Patterns -===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file register the LLHD canonicalization patterns.
//
//===----------------------------------------------------------------------===//

#include "circt/Dialect/LLHD/IR/LLHDOps.h"
#include "mlir/Dialect/StandardOps/IR/Ops.h"
#include "mlir/IR/PatternMatch.h"

using namespace mlir;
using namespace circt;

namespace {
/// Include the patterns defined in the Declarative Rewrite framework.
#include "circt/Dialect/LLHD/IR/LLHDCanonicalization.inc"
} // namespace

void llhd::XorOp::getCanonicalizationPatterns(OwningRewritePatternList &results,
                                              MLIRContext *context) {
  results.insert<XorAllBitsSet>(context);
}

void llhd::NotOp::getCanonicalizationPatterns(OwningRewritePatternList &results,
                                              MLIRContext *context) {
  results.insert<NotOfEq, NotOfNeq>(context);
}

void llhd::EqOp::getCanonicalizationPatterns(OwningRewritePatternList &results,
                                             MLIRContext *context) {
  results.insert<BooleanEqToXor>(context);
}

void llhd::NeqOp::getCanonicalizationPatterns(OwningRewritePatternList &results,
                                              MLIRContext *context) {
  results.insert<BooleanNeqToXor>(context);
}

void llhd::DynExtractSliceOp::getCanonicalizationPatterns(
    OwningRewritePatternList &results, MLIRContext *context) {
  results.insert<DynExtractSliceWithConstantOpStart,
                 DynExtractSliceWithLLHDConstOpStart>(context);
}

void llhd::DynExtractElementOp::getCanonicalizationPatterns(
    OwningRewritePatternList &results, MLIRContext *context) {
  results.insert<DynExtractElementWithConstantOpIndex,
                 DynExtractElementWithLLHDConstOpIndex>(context);
}
