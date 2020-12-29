//===- RTLTypes.cpp - RTL types code defs ---------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implementation logic for RTL data types.
//
//===----------------------------------------------------------------------===//

#include "circt/Dialect/RTL/RTLTypes.h"
#include "circt/Dialect/RTL/RTLDialect.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/Types.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/TypeSwitch.h"

using namespace mlir;
using namespace circt::rtl;

/// Return true if the specified type can be used as an RTL value type, that is
/// the set of types that can be composed together to represent synthesized,
/// hardware but not marker types like InOutType.
bool isRTLValueType(Type type) {
  if (auto intType = type.dyn_cast<IntegerType>())
    return intType.isSignless();

  if (type.isa<ArrayType>())
    return true;

  return false;
}

Type getTypeByField(Type structVal, StringRef fieldName) { return structVal; }

//===----------------------------------------------------------------------===//
// Struct Type
//===----------------------------------------------------------------------===//

namespace circt {
namespace rtl {
static bool operator==(const FieldInfo &a, const FieldInfo &b) {
  return a.name == b.name && a.type == b.type;
}
static llvm::hash_code hash_value(const FieldInfo &fi) {
  return llvm::hash_combine(fi.name, fi.type);
}
} // namespace rtl
} // namespace circt

namespace circt {
namespace rtl {
// llvm::hash_code hash_value(const StructType::StructElement &arg) {
//   return llvm::hash_value(arg.name) ^ mlir::hash_value(arg.type);
// }
} // namespace rtl
} // namespace circt

namespace circt {
namespace rtl {
namespace detail {
// struct StructTypeStorage : mlir::TypeStorage {
//   using KeyTy = ArrayRef<StructType::StructElement>;

//   StructTypeStorage(KeyTy elements)
//       : elements(elements.begin(), elements.end()) {
//   }

//   bool operator==(const KeyTy &key) const { return key == KeyTy(elements); }

//   static llvm::hash_code hashKey(const KeyTy &key) {
//     return llvm::hash_combine_range(key.begin(), key.end());
//   }

//   static StructTypeStorage *construct(TypeStorageAllocator &allocator,
//                                       KeyTy key) {
//     return new (allocator.allocate<StructTypeStorage>())
//     StructTypeStorage(key);
//   }

//   SmallVector<StructType::StructElement, 4> elements;
// };

} // namespace detail
} // namespace rtl
} // namespace circt

// StructType StructType::get(ArrayRef<StructElement> elements,
//                            MLIRContext *context) {
//   return Base::get(context, elements);
// }

// auto StructType::getElements() -> ArrayRef<StructElement> {
//   return getImpl()->elements;
// }

// /// Look up an element by name.  This returns a StructElement.
// Optional<StructType::StructElement> StructType::getElement(StringRef name) {
//   for (const auto &element : getElements()) {
//     if (element.name == name)
//       return element;
//   }
//   return None;
// }

// Type StructType::getElementType(StringRef name) {
//   auto element = getElement(name);
//   return element.hasValue() ? element.getValue().type : RTLType();
// }

//===----------------------------------------------------------------------===//
// ArrayType
//===----------------------------------------------------------------------===//

Type ArrayType::parse(MLIRContext *ctxt, DialectAsmParser &p) {
  SmallVector<int64_t, 2> dims;
  Type inner;
  if (p.parseLess() || p.parseDimensionList(dims, /* allowDynamic */ false) ||
      p.parseType(inner) || p.parseGreater())
    return Type();
  if (dims.size() != 1) {
    p.emitError(p.getNameLoc(), "rtl.array only supports one dimension");
    return Type();
  }

  auto loc = p.getEncodedSourceLoc(p.getCurrentLocation());
  if (failed(verifyConstructionInvariants(loc, inner, dims[0])))
    return Type();

  return get(ctxt, inner, dims[0]);
}

void ArrayType::print(DialectAsmPrinter &p) const {
  p << "array<" << getSize() << "x";
  p.printType(getElementType());
  p << '>';
}

LogicalResult ArrayType::verifyConstructionInvariants(Location loc,
                                                      Type innerType,
                                                      size_t size) {
  if (!isRTLValueType(innerType))
    return emitError(loc, "invalid element for rtl.array type");
  return success();
}

//===----------------------------------------------------------------------===//
// InOutType
//===----------------------------------------------------------------------===//

Type InOutType::parse(MLIRContext *ctxt, DialectAsmParser &p) {
  Type inner;
  if (p.parseLess() || p.parseType(inner) || p.parseGreater())
    return Type();

  auto loc = p.getEncodedSourceLoc(p.getCurrentLocation());
  if (failed(verifyConstructionInvariants(loc, inner)))
    return Type();

  return get(ctxt, inner);
}

void InOutType::print(DialectAsmPrinter &p) const {
  p << "inout<";
  p.printType(getElementType());
  p << '>';
}

LogicalResult InOutType::verifyConstructionInvariants(Location loc,
                                                      Type innerType) {
  if (!isRTLValueType(innerType))
    return emitError(loc, "invalid element for rtl.inout type");
  return success();
}

#define GET_TYPEDEF_CLASSES
#include "circt/Dialect/RTL/RTLTypes.cpp.inc"

/// Parses a type registered to this dialect. Parse out the mnemonic then invoke
/// the tblgen'd type parser dispatcher.
Type RTLDialect::parseType(DialectAsmParser &parser) const {
  llvm::StringRef mnemonic;
  if (parser.parseKeyword(&mnemonic))
    return Type();
  return generatedTypeParser(getContext(), parser, mnemonic);
}

/// Print a type registered to this dialect. Try the tblgen'd type printer
/// dispatcher then fail since all RTL types are defined via ODS.
void RTLDialect::printType(Type type, DialectAsmPrinter &printer) const {
  if (succeeded(generatedTypePrinter(type, printer)))
    return;
  llvm_unreachable("unexpected 'rtl' type");
}