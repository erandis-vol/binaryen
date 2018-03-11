/*
 * Copyright 2016 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//================
// Binaryen C API
//
// The first part of the API lets you create modules and their parts.
//
// The second part of the API lets you perform operations on modules.
//
// The third part of the API lets you provide a general control-flow
//   graph (CFG) as input.
//
// The final part of the API contains miscellaneous utilities like
//   debugging/tracing for the API itself.
//
// ---------------
//
// Thread safety: You can create Expressions in parallel, as they do not
//                refer to global state. BinaryenAddFunction and
//                BinaryenAddFunctionType are also thread-safe, which means
//                that you can create functions and their contents in multiple
//                threads. This is important since functions are where the
//                majority of the work is done.
//                Other methods - creating imports, exports, etc. - are
//                not currently thread-safe (as there is typically no need
//                to parallelize them).
//
//================

#ifndef wasm_binaryen_c_h
#define wasm_binaryen_c_h

#include <stddef.h>
#include <stdint.h>

#include "compiler-support.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// ========== Module Creation ==========
//

// BinaryenIndex
//
// Used for internal indexes and list sizes.

typedef uint32_t BinaryenIndex;

// Core types (call to get the value of each; you can cache them, they
// never change)

typedef uint32_t BinaryenType;

WASM_API BinaryenType BinaryenTypeNone(void);
WASM_API BinaryenType BinaryenTypeInt32(void);
WASM_API BinaryenType BinaryenTypeInt64(void);
WASM_API BinaryenType BinaryenTypeFloat32(void);
WASM_API BinaryenType BinaryenTypeFloat64(void);
WASM_API BinaryenType BinaryenTypeUnreachable(void);
// Not a real type. Used as the last parameter to BinaryenBlock to let
// the API figure out the type instead of providing one.
WASM_API BinaryenType BinaryenTypeAuto(void);

WASM_API WASM_DEPRECATED BinaryenType BinaryenNone(void);
WASM_API WASM_DEPRECATED BinaryenType BinaryenInt32(void);
WASM_API WASM_DEPRECATED BinaryenType BinaryenInt64(void);
WASM_API WASM_DEPRECATED BinaryenType BinaryenFloat32(void);
WASM_API WASM_DEPRECATED BinaryenType BinaryenFloat64(void);
WASM_API WASM_DEPRECATED BinaryenType BinaryenUndefined(void);

// Expression ids (call to get the value of each; you can cache them)

typedef uint32_t BinaryenExpressionId;

WASM_API BinaryenExpressionId BinaryenInvalidId(void);
WASM_API BinaryenExpressionId BinaryenBlockId(void);
WASM_API BinaryenExpressionId BinaryenIfId(void);
WASM_API BinaryenExpressionId BinaryenLoopId(void);
WASM_API BinaryenExpressionId BinaryenBreakId(void);
WASM_API BinaryenExpressionId BinaryenSwitchId(void);
WASM_API BinaryenExpressionId BinaryenCallId(void);
WASM_API BinaryenExpressionId BinaryenCallImportId(void);
WASM_API BinaryenExpressionId BinaryenCallIndirectId(void);
WASM_API BinaryenExpressionId BinaryenGetLocalId(void);
WASM_API BinaryenExpressionId BinaryenSetLocalId(void);
WASM_API BinaryenExpressionId BinaryenGetGlobalId(void);
WASM_API BinaryenExpressionId BinaryenSetGlobalId(void);
WASM_API BinaryenExpressionId BinaryenLoadId(void);
WASM_API BinaryenExpressionId BinaryenStoreId(void);
WASM_API BinaryenExpressionId BinaryenConstId(void);
WASM_API BinaryenExpressionId BinaryenUnaryId(void);
WASM_API BinaryenExpressionId BinaryenBinaryId(void);
WASM_API BinaryenExpressionId BinaryenSelectId(void);
WASM_API BinaryenExpressionId BinaryenDropId(void);
WASM_API BinaryenExpressionId BinaryenReturnId(void);
WASM_API BinaryenExpressionId BinaryenHostId(void);
WASM_API BinaryenExpressionId BinaryenNopId(void);
WASM_API BinaryenExpressionId BinaryenUnreachableId(void);
WASM_API BinaryenExpressionId BinaryenAtomicCmpxchgId(void);
WASM_API BinaryenExpressionId BinaryenAtomicRMWId(void);
WASM_API BinaryenExpressionId BinaryenAtomicWaitId(void);
WASM_API BinaryenExpressionId BinaryenAtomicWakeId(void);

// External kinds (call to get the value of each; you can cache them)

typedef uint32_t BinaryenExternalKind;

WASM_API BinaryenExternalKind BinaryenExternalFunction(void);
WASM_API BinaryenExternalKind BinaryenExternalTable(void);
WASM_API BinaryenExternalKind BinaryenExternalMemory(void);
WASM_API BinaryenExternalKind BinaryenExternalGlobal(void);

// Modules
//
// Modules contain lists of functions, imports, exports, function types. The
// Add* methods create them on a module. The module owns them and will free their
// memory when the module is disposed of.
//
// Expressions are also allocated inside modules, and freed with the module. They
// are not created by Add* methods, since they are not added directly on the
// module, instead, they are arguments to other expressions (and then they are
// the children of that AST node), or to a function (and then they are the body
// of that function).
//
// A module can also contain a function table for indirect calls, a memory,
// and a start method.

typedef void* BinaryenModuleRef;

WASM_API BinaryenModuleRef BinaryenModuleCreate(void);
WASM_API void BinaryenModuleDispose(BinaryenModuleRef module);

// Function types

typedef void* BinaryenFunctionTypeRef;

// Add a new function type. This is thread-safe.
// Note: name can be NULL, in which case we auto-generate a name
WASM_API BinaryenFunctionTypeRef BinaryenAddFunctionType(BinaryenModuleRef module, const char* name, BinaryenType result, BinaryenType* paramTypes, BinaryenIndex numParams);

// Literals. These are passed by value.

// NOTE: takes 16 bytes in memory, with padding between type and value
struct BinaryenLiteral {
  int32_t type;
  union {
    int32_t i32;
    int64_t i64;
    float f32;
    double f64;
  };
};

WASM_API struct BinaryenLiteral BinaryenLiteralInt32(int32_t x);
WASM_API struct BinaryenLiteral BinaryenLiteralInt64(int64_t x);
WASM_API struct BinaryenLiteral BinaryenLiteralFloat32(float x);
WASM_API struct BinaryenLiteral BinaryenLiteralFloat64(double x);
WASM_API struct BinaryenLiteral BinaryenLiteralFloat32Bits(int32_t x);
WASM_API struct BinaryenLiteral BinaryenLiteralFloat64Bits(int64_t x);

// Expressions
//
// Some expressions have a BinaryenOp, which is the more
// specific operation/opcode.
//
// Some expressions have optional parameters, like Return may not
// return a value. You can supply a NULL pointer in those cases.
//
// For more information, see wasm.h

typedef int32_t BinaryenOp;

WASM_API BinaryenOp BinaryenClzInt32(void);
WASM_API BinaryenOp BinaryenCtzInt32(void);
WASM_API BinaryenOp BinaryenPopcntInt32(void);
WASM_API BinaryenOp BinaryenNegFloat32(void);
WASM_API BinaryenOp BinaryenAbsFloat32(void);
WASM_API BinaryenOp BinaryenCeilFloat32(void);
WASM_API BinaryenOp BinaryenFloorFloat32(void);
WASM_API BinaryenOp BinaryenTruncFloat32(void);
WASM_API BinaryenOp BinaryenNearestFloat32(void);
WASM_API BinaryenOp BinaryenSqrtFloat32(void);
WASM_API BinaryenOp BinaryenEqZInt32(void);
WASM_API BinaryenOp BinaryenClzInt64(void);
WASM_API BinaryenOp BinaryenCtzInt64(void);
WASM_API BinaryenOp BinaryenPopcntInt64(void);
WASM_API BinaryenOp BinaryenNegFloat64(void);
WASM_API BinaryenOp BinaryenAbsFloat64(void);
WASM_API BinaryenOp BinaryenCeilFloat64(void);
WASM_API BinaryenOp BinaryenFloorFloat64(void);
WASM_API BinaryenOp BinaryenTruncFloat64(void);
WASM_API BinaryenOp BinaryenNearestFloat64(void);
WASM_API BinaryenOp BinaryenSqrtFloat64(void);
WASM_API BinaryenOp BinaryenEqZInt64(void);
WASM_API BinaryenOp BinaryenExtendSInt32(void);
WASM_API BinaryenOp BinaryenExtendUInt32(void);
WASM_API BinaryenOp BinaryenWrapInt64(void);
WASM_API BinaryenOp BinaryenTruncSFloat32ToInt32(void);
WASM_API BinaryenOp BinaryenTruncSFloat32ToInt64(void);
WASM_API BinaryenOp BinaryenTruncUFloat32ToInt32(void);
WASM_API BinaryenOp BinaryenTruncUFloat32ToInt64(void);
WASM_API BinaryenOp BinaryenTruncSFloat64ToInt32(void);
WASM_API BinaryenOp BinaryenTruncSFloat64ToInt64(void);
WASM_API BinaryenOp BinaryenTruncUFloat64ToInt32(void);
WASM_API BinaryenOp BinaryenTruncUFloat64ToInt64(void);
WASM_API BinaryenOp BinaryenReinterpretFloat32(void);
WASM_API BinaryenOp BinaryenReinterpretFloat64(void);
WASM_API BinaryenOp BinaryenConvertSInt32ToFloat32(void);
WASM_API BinaryenOp BinaryenConvertSInt32ToFloat64(void);
WASM_API BinaryenOp BinaryenConvertUInt32ToFloat32(void);
WASM_API BinaryenOp BinaryenConvertUInt32ToFloat64(void);
WASM_API BinaryenOp BinaryenConvertSInt64ToFloat32(void);
WASM_API BinaryenOp BinaryenConvertSInt64ToFloat64(void);
WASM_API BinaryenOp BinaryenConvertUInt64ToFloat32(void);
WASM_API BinaryenOp BinaryenConvertUInt64ToFloat64(void);
WASM_API BinaryenOp BinaryenPromoteFloat32(void);
WASM_API BinaryenOp BinaryenDemoteFloat64(void);
WASM_API BinaryenOp BinaryenReinterpretInt32(void);
WASM_API BinaryenOp BinaryenReinterpretInt64(void);
WASM_API BinaryenOp BinaryenAddInt32(void);
WASM_API BinaryenOp BinaryenSubInt32(void);
WASM_API BinaryenOp BinaryenMulInt32(void);
WASM_API BinaryenOp BinaryenDivSInt32(void);
WASM_API BinaryenOp BinaryenDivUInt32(void);
WASM_API BinaryenOp BinaryenRemSInt32(void);
WASM_API BinaryenOp BinaryenRemUInt32(void);
WASM_API BinaryenOp BinaryenAndInt32(void);
WASM_API BinaryenOp BinaryenOrInt32(void);
WASM_API BinaryenOp BinaryenXorInt32(void);
WASM_API BinaryenOp BinaryenShlInt32(void);
WASM_API BinaryenOp BinaryenShrUInt32(void);
WASM_API BinaryenOp BinaryenShrSInt32(void);
WASM_API BinaryenOp BinaryenRotLInt32(void);
WASM_API BinaryenOp BinaryenRotRInt32(void);
WASM_API BinaryenOp BinaryenEqInt32(void);
WASM_API BinaryenOp BinaryenNeInt32(void);
WASM_API BinaryenOp BinaryenLtSInt32(void);
WASM_API BinaryenOp BinaryenLtUInt32(void);
WASM_API BinaryenOp BinaryenLeSInt32(void);
WASM_API BinaryenOp BinaryenLeUInt32(void);
WASM_API BinaryenOp BinaryenGtSInt32(void);
WASM_API BinaryenOp BinaryenGtUInt32(void);
WASM_API BinaryenOp BinaryenGeSInt32(void);
WASM_API BinaryenOp BinaryenGeUInt32(void);
WASM_API BinaryenOp BinaryenAddInt64(void);
WASM_API BinaryenOp BinaryenSubInt64(void);
WASM_API BinaryenOp BinaryenMulInt64(void);
WASM_API BinaryenOp BinaryenDivSInt64(void);
WASM_API BinaryenOp BinaryenDivUInt64(void);
WASM_API BinaryenOp BinaryenRemSInt64(void);
WASM_API BinaryenOp BinaryenRemUInt64(void);
WASM_API BinaryenOp BinaryenAndInt64(void);
WASM_API BinaryenOp BinaryenOrInt64(void);
WASM_API BinaryenOp BinaryenXorInt64(void);
WASM_API BinaryenOp BinaryenShlInt64(void);
WASM_API BinaryenOp BinaryenShrUInt64(void);
WASM_API BinaryenOp BinaryenShrSInt64(void);
WASM_API BinaryenOp BinaryenRotLInt64(void);
WASM_API BinaryenOp BinaryenRotRInt64(void);
WASM_API BinaryenOp BinaryenEqInt64(void);
WASM_API BinaryenOp BinaryenNeInt64(void);
WASM_API BinaryenOp BinaryenLtSInt64(void);
WASM_API BinaryenOp BinaryenLtUInt64(void);
WASM_API BinaryenOp BinaryenLeSInt64(void);
WASM_API BinaryenOp BinaryenLeUInt64(void);
WASM_API BinaryenOp BinaryenGtSInt64(void);
WASM_API BinaryenOp BinaryenGtUInt64(void);
WASM_API BinaryenOp BinaryenGeSInt64(void);
WASM_API BinaryenOp BinaryenGeUInt64(void);
WASM_API BinaryenOp BinaryenAddFloat32(void);
WASM_API BinaryenOp BinaryenSubFloat32(void);
WASM_API BinaryenOp BinaryenMulFloat32(void);
WASM_API BinaryenOp BinaryenDivFloat32(void);
WASM_API BinaryenOp BinaryenCopySignFloat32(void);
WASM_API BinaryenOp BinaryenMinFloat32(void);
WASM_API BinaryenOp BinaryenMaxFloat32(void);
WASM_API BinaryenOp BinaryenEqFloat32(void);
WASM_API BinaryenOp BinaryenNeFloat32(void);
WASM_API BinaryenOp BinaryenLtFloat32(void);
WASM_API BinaryenOp BinaryenLeFloat32(void);
WASM_API BinaryenOp BinaryenGtFloat32(void);
WASM_API BinaryenOp BinaryenGeFloat32(void);
WASM_API BinaryenOp BinaryenAddFloat64(void);
WASM_API BinaryenOp BinaryenSubFloat64(void);
WASM_API BinaryenOp BinaryenMulFloat64(void);
WASM_API BinaryenOp BinaryenDivFloat64(void);
WASM_API BinaryenOp BinaryenCopySignFloat64(void);
WASM_API BinaryenOp BinaryenMinFloat64(void);
WASM_API BinaryenOp BinaryenMaxFloat64(void);
WASM_API BinaryenOp BinaryenEqFloat64(void);
WASM_API BinaryenOp BinaryenNeFloat64(void);
WASM_API BinaryenOp BinaryenLtFloat64(void);
WASM_API BinaryenOp BinaryenLeFloat64(void);
WASM_API BinaryenOp BinaryenGtFloat64(void);
WASM_API BinaryenOp BinaryenGeFloat64(void);
WASM_API BinaryenOp BinaryenPageSize(void);
WASM_API BinaryenOp BinaryenCurrentMemory(void);
WASM_API BinaryenOp BinaryenGrowMemory(void);
WASM_API BinaryenOp BinaryenHasFeature(void);
WASM_API BinaryenOp BinaryenAtomicRMWAdd(void);
WASM_API BinaryenOp BinaryenAtomicRMWSub(void);
WASM_API BinaryenOp BinaryenAtomicRMWAnd(void);
WASM_API BinaryenOp BinaryenAtomicRMWOr(void);
WASM_API BinaryenOp BinaryenAtomicRMWXor(void);
WASM_API BinaryenOp BinaryenAtomicRMWXchg(void);

typedef void* BinaryenExpressionRef;

// Block: name can be NULL. Specifying BinaryenUndefined() as the 'type'
//        parameter indicates that the block's type shall be figured out
//        automatically instead of explicitly providing it. This conforms
//        to the behavior before the 'type' parameter has been introduced.
WASM_API BinaryenExpressionRef BinaryenBlock(BinaryenModuleRef module, const char* name, BinaryenExpressionRef* children, BinaryenIndex numChildren, BinaryenType type);
// If: ifFalse can be NULL
WASM_API BinaryenExpressionRef BinaryenIf(BinaryenModuleRef module, BinaryenExpressionRef condition, BinaryenExpressionRef ifTrue, BinaryenExpressionRef ifFalse);
WASM_API BinaryenExpressionRef BinaryenLoop(BinaryenModuleRef module, const char* in, BinaryenExpressionRef body);
// Break: value and condition can be NULL
WASM_API BinaryenExpressionRef BinaryenBreak(BinaryenModuleRef module, const char* name, BinaryenExpressionRef condition, BinaryenExpressionRef value);
// Switch: value can be NULL
WASM_API BinaryenExpressionRef BinaryenSwitch(BinaryenModuleRef module, const char **names, BinaryenIndex numNames, const char* defaultName, BinaryenExpressionRef condition, BinaryenExpressionRef value);
// Call, CallImport: Note the 'returnType' parameter. You must declare the
//                   type returned by the function being called, as that
//                   function might not have been created yet, so we don't
//                   know what it is.
//                   Also note that WebAssembly does not differentiate
//                   between Call and CallImport, but Binaryen does, so you
//                   must use CallImport if calling an import, and vice versa.
WASM_API BinaryenExpressionRef BinaryenCall(BinaryenModuleRef module, const char *target, BinaryenExpressionRef* operands, BinaryenIndex numOperands, BinaryenType returnType);
WASM_API BinaryenExpressionRef BinaryenCallImport(BinaryenModuleRef module, const char *target, BinaryenExpressionRef* operands, BinaryenIndex numOperands, BinaryenType returnType);
WASM_API BinaryenExpressionRef BinaryenCallIndirect(BinaryenModuleRef module, BinaryenExpressionRef target, BinaryenExpressionRef* operands, BinaryenIndex numOperands, const char* type);
// GetLocal: Note the 'type' parameter. It might seem redundant, since the
//           local at that index must have a type. However, this API lets you
//           build code "top-down": create a node, then its parents, and so
//           on, and finally create the function at the end. (Note that in fact
//           you do not mention a function when creating ExpressionRefs, only
//           a module.) And since GetLocal is a leaf node, we need to be told
//           its type. (Other nodes detect their type either from their
//           type or their opcode, or failing that, their children. But
//           GetLocal has no children, it is where a "stream" of type info
//           begins.)
//           Note also that the index of a local can refer to a param or
//           a var, that is, either a parameter to the function or a variable
//           declared when you call BinaryenAddFunction. See BinaryenAddFunction
//           for more details.
WASM_API BinaryenExpressionRef BinaryenGetLocal(BinaryenModuleRef module, BinaryenIndex index, BinaryenType type);
WASM_API BinaryenExpressionRef BinaryenSetLocal(BinaryenModuleRef module, BinaryenIndex index, BinaryenExpressionRef value);
WASM_API BinaryenExpressionRef BinaryenTeeLocal(BinaryenModuleRef module, BinaryenIndex index, BinaryenExpressionRef value);
WASM_API BinaryenExpressionRef BinaryenGetGlobal(BinaryenModuleRef module, const char *name, BinaryenType type);
WASM_API BinaryenExpressionRef BinaryenSetGlobal(BinaryenModuleRef module, const char *name, BinaryenExpressionRef value);
// Load: align can be 0, in which case it will be the natural alignment (equal to bytes)
WASM_API BinaryenExpressionRef BinaryenLoad(BinaryenModuleRef module, uint32_t bytes, int8_t signed_, uint32_t offset, uint32_t align, BinaryenType type, BinaryenExpressionRef ptr);
// Store: align can be 0, in which case it will be the natural alignment (equal to bytes)
WASM_API BinaryenExpressionRef BinaryenStore(BinaryenModuleRef module, uint32_t bytes, uint32_t offset, uint32_t align, BinaryenExpressionRef ptr, BinaryenExpressionRef value, BinaryenType type);
WASM_API BinaryenExpressionRef BinaryenConst(BinaryenModuleRef module, struct BinaryenLiteral value);
WASM_API BinaryenExpressionRef BinaryenUnary(BinaryenModuleRef module, BinaryenOp op, BinaryenExpressionRef value);
WASM_API BinaryenExpressionRef BinaryenBinary(BinaryenModuleRef module, BinaryenOp op, BinaryenExpressionRef left, BinaryenExpressionRef right);
WASM_API BinaryenExpressionRef BinaryenSelect(BinaryenModuleRef module, BinaryenExpressionRef condition, BinaryenExpressionRef ifTrue, BinaryenExpressionRef ifFalse);
WASM_API BinaryenExpressionRef BinaryenDrop(BinaryenModuleRef module, BinaryenExpressionRef value);
// Return: value can be NULL
WASM_API BinaryenExpressionRef BinaryenReturn(BinaryenModuleRef module, BinaryenExpressionRef value);
// Host: name may be NULL
WASM_API BinaryenExpressionRef BinaryenHost(BinaryenModuleRef module, BinaryenOp op, const char* name, BinaryenExpressionRef* operands, BinaryenIndex numOperands);
WASM_API BinaryenExpressionRef BinaryenNop(BinaryenModuleRef module);
WASM_API BinaryenExpressionRef BinaryenUnreachable(BinaryenModuleRef module);
WASM_API BinaryenExpressionRef BinaryenAtomicLoad(BinaryenModuleRef module, uint32_t bytes, uint32_t offset, BinaryenType type, BinaryenExpressionRef ptr);
WASM_API BinaryenExpressionRef BinaryenAtomicStore(BinaryenModuleRef module, uint32_t bytes, uint32_t offset, BinaryenExpressionRef ptr, BinaryenExpressionRef value, BinaryenType type);
WASM_API BinaryenExpressionRef BinaryenAtomicRMW(BinaryenModuleRef module, BinaryenOp op, BinaryenIndex bytes, BinaryenIndex offset, BinaryenExpressionRef ptr, BinaryenExpressionRef value, BinaryenType type);
WASM_API BinaryenExpressionRef BinaryenAtomicCmpxchg(BinaryenModuleRef module, BinaryenIndex bytes, BinaryenIndex offset, BinaryenExpressionRef ptr, BinaryenExpressionRef expected, BinaryenExpressionRef replacement, BinaryenType type);
WASM_API BinaryenExpressionRef BinaryenAtomicWait(BinaryenModuleRef module, BinaryenExpressionRef ptr, BinaryenExpressionRef expected, BinaryenExpressionRef timeout, BinaryenType type);
WASM_API BinaryenExpressionRef BinaryenAtomicWake(BinaryenModuleRef module, BinaryenExpressionRef ptr, BinaryenExpressionRef wakeCount);

// Gets the id (kind) of the specified expression.
WASM_API BinaryenExpressionId BinaryenExpressionGetId(BinaryenExpressionRef expr);
// Gets the type of the specified expression.
WASM_API BinaryenType BinaryenExpressionGetType(BinaryenExpressionRef expr);
// Prints an expression to stdout. Useful for debugging.
WASM_API void BinaryenExpressionPrint(BinaryenExpressionRef expr);

// Gets the name of the specified `Block` expression. May be `NULL`.
WASM_API const char* BinaryenBlockGetName(BinaryenExpressionRef expr);
// Gets the number of nested child expressions within the specified `Block` expression.
WASM_API BinaryenIndex BinaryenBlockGetNumChildren(BinaryenExpressionRef expr);
// Gets the nested child expression at the specified index within the specified `Block` expression.
WASM_API BinaryenExpressionRef BinaryenBlockGetChild(BinaryenExpressionRef expr, BinaryenIndex index);

// Gets the nested condition expression within the specified `If` expression.
WASM_API BinaryenExpressionRef BinaryenIfGetCondition(BinaryenExpressionRef expr);
// Gets the nested ifTrue expression within the specified `If` expression.
WASM_API BinaryenExpressionRef BinaryenIfGetIfTrue(BinaryenExpressionRef expr);
// Gets the nested ifFalse expression within the specified `If` expression.
WASM_API BinaryenExpressionRef BinaryenIfGetIfFalse(BinaryenExpressionRef expr);

// Gets the name of the specified `Loop` expression. May be `NULL`.
WASM_API const char* BinaryenLoopGetName(BinaryenExpressionRef expr);
// Gets the nested body expression within the specified `Loop` expression.
WASM_API BinaryenExpressionRef BinaryenLoopGetBody(BinaryenExpressionRef expr);

// Gets the name of the specified `Break` expression. May be `NULL`.
WASM_API const char* BinaryenBreakGetName(BinaryenExpressionRef expr);
// Gets the nested condition expression within the specified `Break` expression. Returns `NULL` if this is a `br` and not a `br_if`.
WASM_API BinaryenExpressionRef BinaryenBreakGetCondition(BinaryenExpressionRef expr);
// Gets the nested value expression within the specified `Break` expression. May be `NULL`.
WASM_API BinaryenExpressionRef BinaryenBreakGetValue(BinaryenExpressionRef expr);

// Gets the number of names within the specified `Switch` expression.
WASM_API BinaryenIndex BinaryenSwitchGetNumNames(BinaryenExpressionRef expr);
// Gets the name at the specified index within the specified `Switch` expression.
WASM_API const char* BinaryenSwitchGetName(BinaryenExpressionRef expr, BinaryenIndex index);
// Gets the default name of the specified `Switch` expression.
WASM_API const char* BinaryenSwitchGetDefaultName(BinaryenExpressionRef expr);
// Gets the nested condition expression within the specified `Switch` expression.
WASM_API BinaryenExpressionRef BinaryenSwitchGetCondition(BinaryenExpressionRef expr);
// Gets the nested value expression within the specifiedd `Switch` expression. May be `NULL`.
WASM_API BinaryenExpressionRef BinaryenSwitchGetValue(BinaryenExpressionRef expr);

// Gets the name of the target of the specified `Call` expression.
WASM_API const char* BinaryenCallGetTarget(BinaryenExpressionRef expr);
// Gets the number of nested operand expressions within the specified `Call` expression.
WASM_API BinaryenIndex BinaryenCallGetNumOperands(BinaryenExpressionRef expr);
// Gets the nested operand expression at the specified index within the specified `Call` expression.
WASM_API BinaryenExpressionRef BinaryenCallGetOperand(BinaryenExpressionRef expr, BinaryenIndex index);

// Gets the name of the target of the specified `CallImport` expression.
WASM_API const char* BinaryenCallImportGetTarget(BinaryenExpressionRef expr);
// Gets the number of nested operand expressions within the specified `CallImport` expression.
WASM_API BinaryenIndex BinaryenCallImportGetNumOperands(BinaryenExpressionRef expr);
// Gets the nested operand expression at the specified index within the specified `CallImport` expression.
WASM_API BinaryenExpressionRef BinaryenCallImportGetOperand(BinaryenExpressionRef expr, BinaryenIndex index);

// Gets the nested target expression of the specified `CallIndirect` expression.
WASM_API BinaryenExpressionRef BinaryenCallIndirectGetTarget(BinaryenExpressionRef expr);
// Gets the number of nested operand expressions within the specified `CallIndirect` expression.
WASM_API BinaryenIndex BinaryenCallIndirectGetNumOperands(BinaryenExpressionRef expr);
// Gets the nested operand expression at the specified index within the specified `CallIndirect` expression.
WASM_API BinaryenExpressionRef BinaryenCallIndirectGetOperand(BinaryenExpressionRef expr, BinaryenIndex index);

// Gets the index of the specified `GetLocal` expression.
WASM_API BinaryenIndex BinaryenGetLocalGetIndex(BinaryenExpressionRef expr);

// Tests if the specified `SetLocal` expression performs a `tee_local` instead of a `set_local`.
WASM_API int BinaryenSetLocalIsTee(BinaryenExpressionRef expr);
// Gets the index of the specified `SetLocal` expression.
WASM_API BinaryenIndex BinaryenSetLocalGetIndex(BinaryenExpressionRef expr);
// Gets the nested value expression within the specified `SetLocal` expression.
WASM_API BinaryenExpressionRef BinaryenSetLocalGetValue(BinaryenExpressionRef expr);

// Gets the name of the specified `GetGlobal` expression.
WASM_API const char* BinaryenGetGlobalGetName(BinaryenExpressionRef expr);

// Gets the name of the specified `SetGlobal` expression.
WASM_API const char* BinaryenSetGlobalGetName(BinaryenExpressionRef expr);
// Gets the nested value expression within the specified `SetLocal` expression.
WASM_API BinaryenExpressionRef BinaryenSetGlobalGetValue(BinaryenExpressionRef expr);

// Gets the operator of the specified `Host` expression.
WASM_API BinaryenOp BinaryenHostGetOp(BinaryenExpressionRef expr);
// Gets the name operand of the specified `Host` expression. May be `NULL`.
WASM_API const char* BinaryenHostGetNameOperand(BinaryenExpressionRef expr);
// Gets the number of nested operand expressions within the specified `Host` expression.
WASM_API BinaryenIndex BinaryenHostGetNumOperands(BinaryenExpressionRef expr);
// Gets the nested operand expression at the specified index within the specified `Host` expression.
WASM_API BinaryenExpressionRef BinaryenHostGetOperand(BinaryenExpressionRef expr, BinaryenIndex index);

// Tests if the specified `Load` expression is atomic.
WASM_API int BinaryenLoadIsAtomic(BinaryenExpressionRef expr);
// Tests if the specified `Load` expression is signed.
WASM_API int BinaryenLoadIsSigned(BinaryenExpressionRef expr);
// Gets the offset of the specified `Load` expression.
WASM_API uint32_t BinaryenLoadGetOffset(BinaryenExpressionRef expr);
// Gets the byte size of the specified `Load` expression.
WASM_API uint32_t BinaryenLoadGetBytes(BinaryenExpressionRef expr);
// Gets the alignment of the specified `Load` expression.
WASM_API uint32_t BinaryenLoadGetAlign(BinaryenExpressionRef expr);
// Gets the nested pointer expression within the specified `Load` expression.
WASM_API BinaryenExpressionRef BinaryenLoadGetPtr(BinaryenExpressionRef expr);

// Tests if the specified `Store` expression is atomic.
WASM_API int BinaryenStoreIsAtomic(BinaryenExpressionRef expr);
// Gets the byte size of the specified `Store` expression.
WASM_API uint32_t BinaryenStoreGetBytes(BinaryenExpressionRef expr);
// Gets the offset of the specified store expression.
WASM_API uint32_t BinaryenStoreGetOffset(BinaryenExpressionRef expr);
// Gets the alignment of the specified `Store` expression.
WASM_API uint32_t BinaryenStoreGetAlign(BinaryenExpressionRef expr);
// Gets the nested pointer expression within the specified `Store` expression.
WASM_API BinaryenExpressionRef BinaryenStoreGetPtr(BinaryenExpressionRef expr);
// Gets the nested value expression within the specified `Store` expression.
WASM_API BinaryenExpressionRef BinaryenStoreGetValue(BinaryenExpressionRef expr);

// Gets the 32-bit integer value of the specified `Const` expression.
WASM_API int32_t BinaryenConstGetValueI32(BinaryenExpressionRef expr);
// Gets the 64-bit integer value of the specified `Const` expression.
WASM_API int64_t BinaryenConstGetValueI64(BinaryenExpressionRef expr);
// Gets the low 32-bits of a 64-bit integer value of the specified `Const` expression. Useful where I64 returning exports are illegal, i.e. binaryen.js.
WASM_API int32_t BinaryenConstGetValueI64Low(BinaryenExpressionRef expr);
// Gets the high 32-bits of a 64-bit integer value of the specified `Const` expression. Useful where I64 returning exports are illegal, i.e. binaryen.js.
WASM_API int32_t BinaryenConstGetValueI64High(BinaryenExpressionRef expr);
// Gets the 32-bit float value of the specified `Const` expression.
WASM_API float BinaryenConstGetValueF32(BinaryenExpressionRef expr);
// Gets the 64-bit float value of the specified `Const` expression.
WASM_API double BinaryenConstGetValueF64(BinaryenExpressionRef expr);

// Gets the operator of the specified `Unary` expression.
WASM_API BinaryenOp BinaryenUnaryGetOp(BinaryenExpressionRef expr);
// Gets the nested value expression within the specified `Unary` expression.
WASM_API BinaryenExpressionRef BinaryenUnaryGetValue(BinaryenExpressionRef expr);

// Gets the operator of the specified `Binary` expression.
WASM_API BinaryenOp BinaryenBinaryGetOp(BinaryenExpressionRef expr);
// Gets the nested left expression within the specified `Binary` expression.
WASM_API BinaryenExpressionRef BinaryenBinaryGetLeft(BinaryenExpressionRef expr);
// Gets the nested right expression within the specified `Binary` expression.
WASM_API BinaryenExpressionRef BinaryenBinaryGetRight(BinaryenExpressionRef expr);

// Gets the nested ifTrue expression within the specified `Select` expression.
WASM_API BinaryenExpressionRef BinaryenSelectGetIfTrue(BinaryenExpressionRef expr);
// Gets the nested ifFalse expression within the specified `Select` expression.
WASM_API BinaryenExpressionRef BinaryenSelectGetIfFalse(BinaryenExpressionRef expr);
// Gets the nested condition expression within the specified `Select` expression.
WASM_API BinaryenExpressionRef BinaryenSelectGetCondition(BinaryenExpressionRef expr);

// Gets the nested value expression within the specified `Drop` expression.
WASM_API BinaryenExpressionRef BinaryenDropGetValue(BinaryenExpressionRef expr);

// Gets the nested value expression within the specified `Return` expression.
WASM_API BinaryenExpressionRef BinaryenReturnGetValue(BinaryenExpressionRef expr);

// Gets the operator of the specified `AtomicRMW` expression.
WASM_API BinaryenOp BinaryenAtomicRMWGetOp(BinaryenExpressionRef expr);
// Gets the byte size of the specified `AtomicRMW` expression.
WASM_API uint32_t BinaryenAtomicRMWGetBytes(BinaryenExpressionRef expr);
// Gets the offset of the specified `AtomicRMW` expression.
WASM_API uint32_t BinaryenAtomicRMWGetOffset(BinaryenExpressionRef expr);
// Gets the nested pointer expression within the specified `AtomicRMW` expression.
WASM_API BinaryenExpressionRef BinaryenAtomicRMWGetPtr(BinaryenExpressionRef expr);
// Gets the nested value expression within the specified `AtomicRMW` expression.
WASM_API BinaryenExpressionRef BinaryenAtomicRMWGetValue(BinaryenExpressionRef expr);

// Gets the byte size of the specified `AtomicCmpxchg` expression.
WASM_API uint32_t BinaryenAtomicCmpxchgGetBytes(BinaryenExpressionRef expr);
// Gets the offset of the specified `AtomicCmpxchg` expression.
WASM_API uint32_t BinaryenAtomicCmpxchgGetOffset(BinaryenExpressionRef expr);
// Gets the nested pointer expression within the specified `AtomicCmpxchg` expression.
WASM_API BinaryenExpressionRef BinaryenAtomicCmpxchgGetPtr(BinaryenExpressionRef expr);
// Gets the nested expected value expression within the specified `AtomicCmpxchg` expression.
WASM_API BinaryenExpressionRef BinaryenAtomicCmpxchgGetExpected(BinaryenExpressionRef expr);
// Gets the nested replacement value expression within the specified `AtomicCmpxchg` expression.
WASM_API BinaryenExpressionRef BinaryenAtomicCmpxchgGetReplacement(BinaryenExpressionRef expr);

// Gets the nested pointer expression within the specified `AtomicWait` expression.
WASM_API BinaryenExpressionRef BinaryenAtomicWaitGetPtr(BinaryenExpressionRef expr);
// Gets the nested expected value expression within the specified `AtomicWait` expression.
WASM_API BinaryenExpressionRef BinaryenAtomicWaitGetExpected(BinaryenExpressionRef expr);
// Gets the nested timeout expression within the specified `AtomicWait` expression.
WASM_API BinaryenExpressionRef BinaryenAtomicWaitGetTimeout(BinaryenExpressionRef expr);
// Gets the expected type of the specified `AtomicWait` expression.
WASM_API BinaryenType BinaryenAtomicWaitGetExpectedType(BinaryenExpressionRef expr);

// Gets the nested pointer expression within the specified `AtomicWake` expression.
WASM_API BinaryenExpressionRef BinaryenAtomicWakeGetPtr(BinaryenExpressionRef expr);
// Gets the nested wake count expression within the specified `AtomicWake` expression.
WASM_API BinaryenExpressionRef BinaryenAtomicWakeGetWakeCount(BinaryenExpressionRef expr);

// Functions

typedef void* BinaryenFunctionRef;

// Adds a function to the module. This is thread-safe.
// @varTypes: the types of variables. In WebAssembly, vars share
//            an index space with params. In other words, params come from
//            the function type, and vars are provided in this call, and
//            together they are all the locals. The order is first params
//            and then vars, so if you have one param it will be at index
//            0 (and written $0), and if you also have 2 vars they will be
//            at indexes 1 and 2, etc., that is, they share an index space.
WASM_API BinaryenFunctionRef BinaryenAddFunction(BinaryenModuleRef module, const char* name, BinaryenFunctionTypeRef type, BinaryenType* varTypes, BinaryenIndex numVarTypes, BinaryenExpressionRef body);

// Gets a function reference by name.
WASM_API BinaryenFunctionRef BinaryenGetFunction(BinaryenModuleRef module, const char* name);

// Removes a function by name.
WASM_API void BinaryenRemoveFunction(BinaryenModuleRef module, const char* name);

// Imports

typedef void* BinaryenImportRef;

WASM_API WASM_DEPRECATED BinaryenImportRef BinaryenAddImport(BinaryenModuleRef module, const char* internalName, const char* externalModuleName, const char *externalBaseName, BinaryenFunctionTypeRef type);
WASM_API BinaryenImportRef BinaryenAddFunctionImport(BinaryenModuleRef module, const char* internalName, const char* externalModuleName, const char *externalBaseName, BinaryenFunctionTypeRef functionType);
WASM_API BinaryenImportRef BinaryenAddTableImport(BinaryenModuleRef module, const char* internalName, const char* externalModuleName, const char *externalBaseName);
WASM_API BinaryenImportRef BinaryenAddMemoryImport(BinaryenModuleRef module, const char* internalName, const char* externalModuleName, const char *externalBaseName);
WASM_API BinaryenImportRef BinaryenAddGlobalImport(BinaryenModuleRef module, const char* internalName, const char* externalModuleName, const char *externalBaseName, BinaryenType globalType);
WASM_API void BinaryenRemoveImport(BinaryenModuleRef module, const char* internalName);

// Exports

typedef void* BinaryenExportRef;

WASM_API WASM_DEPRECATED BinaryenExportRef BinaryenAddExport(BinaryenModuleRef module, const char* internalName, const char* externalName);
WASM_API BinaryenExportRef BinaryenAddFunctionExport(BinaryenModuleRef module, const char* internalName, const char* externalName);
WASM_API BinaryenExportRef BinaryenAddTableExport(BinaryenModuleRef module, const char* internalName, const char* externalName);
WASM_API BinaryenExportRef BinaryenAddMemoryExport(BinaryenModuleRef module, const char* internalName, const char* externalName);
WASM_API BinaryenExportRef BinaryenAddGlobalExport(BinaryenModuleRef module, const char* internalName, const char* externalName);
WASM_API void BinaryenRemoveExport(BinaryenModuleRef module, const char* externalName);

// Globals

typedef void* BinaryenGlobalRef;

WASM_API BinaryenGlobalRef BinaryenAddGlobal(BinaryenModuleRef module, const char* name, BinaryenType type, int8_t mutable_, BinaryenExpressionRef init);

// Function table. One per module

WASM_API void BinaryenSetFunctionTable(BinaryenModuleRef module, BinaryenFunctionRef* funcs, BinaryenIndex numFuncs);

// Memory. One per module

// Each segment has data in segments, a start offset in segmentOffsets, and a size in segmentSizes.
// exportName can be NULL
WASM_API void BinaryenSetMemory(BinaryenModuleRef module, BinaryenIndex initial, BinaryenIndex maximum, const char* exportName, const char **segments, BinaryenExpressionRef* segmentOffsets, BinaryenIndex* segmentSizes, BinaryenIndex numSegments);

// Start function. One per module

WASM_API void BinaryenSetStart(BinaryenModuleRef module, BinaryenFunctionRef start);

//
// ========== Module Operations ==========
//

// Parse a module in s-expression text format
WASM_API BinaryenModuleRef BinaryenModuleParse(const char* text);

// Print a module to stdout in s-expression text format. Useful for debugging.
WASM_API void BinaryenModulePrint(BinaryenModuleRef module);

// Print a module to stdout in asm.js syntax.
WASM_API void BinaryenModulePrintAsmjs(BinaryenModuleRef module);

// Validate a module, showing errors on problems.
//  @return 0 if an error occurred, 1 if validated succesfully
WASM_API int BinaryenModuleValidate(BinaryenModuleRef module);

// Runs the standard optimization passes on the module. Uses the currently set
// global optimize and shrink level.
WASM_API void BinaryenModuleOptimize(BinaryenModuleRef module);

// Gets the currently set optimize level. Applies to all modules, globally.
// 0, 1, 2 correspond to -O0, -O1, -O2 (default), etc.
WASM_API int BinaryenGetOptimizeLevel();

// Sets the optimization level to use. Applies to all modules, globally.
// 0, 1, 2 correspond to -O0, -O1, -O2 (default), etc.
WASM_API void BinaryenSetOptimizeLevel(int level);

// Gets the currently set shrink level. Applies to all modules, globally.
// 0, 1, 2 correspond to -O0, -Os (default), -Oz.
WASM_API int BinaryenGetShrinkLevel();

// Sets the shrink level to use. Applies to all modules, globally.
// 0, 1, 2 correspond to -O0, -Os (default), -Oz.
WASM_API void BinaryenSetShrinkLevel(int level);

// Gets whether generating debug information is currently enabled or not.
// Applies to all modules, globally.
WASM_API int BinaryenGetDebugInfo();

// Enables or disables debug information in emitted binaries.
// Applies to all modules, globally.
WASM_API void BinaryenSetDebugInfo(int on);

// Runs the specified passes on the module. Uses the currently set global
// optimize and shrink level.
WASM_API void BinaryenModuleRunPasses(BinaryenModuleRef module, const char **passes, BinaryenIndex numPasses);

// Auto-generate drop() operations where needed. This lets you generate code without
// worrying about where they are needed. (It is more efficient to do it yourself,
// but simpler to use autodrop).
WASM_API void BinaryenModuleAutoDrop(BinaryenModuleRef module);

// Serialize a module into binary form. Uses the currently set global debugInfo option.
// @return how many bytes were written. This will be less than or equal to outputSize
WASM_API size_t BinaryenModuleWrite(BinaryenModuleRef module, char* output, size_t outputSize);

typedef struct BinaryenBufferSizes {
  size_t outputBytes;
  size_t sourceMapBytes;
} BinaryenBufferSizes;

// Serialize a module into binary form including its source map. Uses the currently set
// global debugInfo option.
// @returns how many bytes were written. This will be less than or equal to outputSize
WASM_API BinaryenBufferSizes BinaryenModuleWriteWithSourceMap(BinaryenModuleRef module, const char* url, char* output, size_t outputSize, char* sourceMap, size_t sourceMapSize);

// Result structure of BinaryenModuleAllocateAndWrite. Contained buffers have been allocated
// using malloc() and the user is expected to free() them manually once not needed anymore.
typedef struct BinaryenModuleAllocateAndWriteResult {
  void* binary;
  size_t binaryBytes;
  char* sourceMap;
} BinaryenModuleAllocateAndWriteResult;

// Serializes a module into binary form, optionally including its source map if
// sourceMapUrl has been specified. Uses the currently set global debugInfo option.
// Differs from BinaryenModuleWrite in that it implicitly allocates appropriate buffers
// using malloc(), and expects the user to free() them manually once not needed anymore.
WASM_API BinaryenModuleAllocateAndWriteResult BinaryenModuleAllocateAndWrite(BinaryenModuleRef module, const char* sourceMapUrl);

// Deserialize a module from binary form.
WASM_API BinaryenModuleRef BinaryenModuleRead(char* input, size_t inputSize);

// Execute a module in the Binaryen interpreter. This will create an instance of
// the module, run it in the interpreter - which means running the start method -
// and then destroying the instance.
WASM_API void BinaryenModuleInterpret(BinaryenModuleRef module);

// Adds a debug info file name to the module and returns its index.
WASM_API BinaryenIndex BinaryenModuleAddDebugInfoFileName(BinaryenModuleRef module, const char* filename);

// Gets the name of the debug info file at the specified index. Returns `NULL` if it
// does not exist.
WASM_API const char* BinaryenModuleGetDebugInfoFileName(BinaryenModuleRef module, BinaryenIndex index);

//
// ======== FunctionType Operations ========
//

// Gets the name of the specified `FunctionType`.
WASM_API const char* BinaryenFunctionTypeGetName(BinaryenFunctionTypeRef ftype);
// Gets the number of parameters of the specified `FunctionType`.
WASM_API BinaryenIndex BinaryenFunctionTypeGetNumParams(BinaryenFunctionTypeRef ftype);
// Gets the type of the parameter at the specified index of the specified `FunctionType`.
WASM_API BinaryenType BinaryenFunctionTypeGetParam(BinaryenFunctionTypeRef ftype, BinaryenIndex index);
// Gets the result type of the specified `FunctionType`.
WASM_API BinaryenType BinaryenFunctionTypeGetResult(BinaryenFunctionTypeRef ftype);

//
// ========== Function Operations ==========
//

// Gets the name of the specified `Function`.
WASM_API const char* BinaryenFunctionGetName(BinaryenFunctionRef func);
// Gets the name of the `FunctionType` associated with the specified `Function`. May be `NULL` if the signature is implicit.
WASM_API const char* BinaryenFunctionGetType(BinaryenFunctionRef func);
// Gets the number of parameters of the specified `Function`.
WASM_API BinaryenIndex BinaryenFunctionGetNumParams(BinaryenFunctionRef func);
// Gets the type of the parameter at the specified index of the specified `Function`.
WASM_API BinaryenType BinaryenFunctionGetParam(BinaryenFunctionRef func, BinaryenIndex index);
// Gets the result type of the specified `Function`.
WASM_API BinaryenType BinaryenFunctionGetResult(BinaryenFunctionRef func);
// Gets the number of additional locals within the specified `Function`.
WASM_API BinaryenIndex BinaryenFunctionGetNumVars(BinaryenFunctionRef func);
// Gets the type of the additional local at the specified index within the specified `Function`.
WASM_API BinaryenType BinaryenFunctionGetVar(BinaryenFunctionRef func, BinaryenIndex index);
// Gets the body of the specified `Function`.
WASM_API BinaryenExpressionRef BinaryenFunctionGetBody(BinaryenFunctionRef func);

// Runs the standard optimization passes on the function. Uses the currently set
// global optimize and shrink level.
WASM_API void BinaryenFunctionOptimize(BinaryenFunctionRef func, BinaryenModuleRef module);

// Runs the specified passes on the function. Uses the currently set global
// optimize and shrink level.
WASM_API void BinaryenFunctionRunPasses(BinaryenFunctionRef func, BinaryenModuleRef module, const char **passes, BinaryenIndex numPasses);

// Sets the debug location of the specified `Expression` within the specified `Function`.
WASM_API void BinaryenFunctionSetDebugLocation(BinaryenFunctionRef func, BinaryenExpressionRef expr, BinaryenIndex fileIndex, BinaryenIndex lineNumber, BinaryenIndex columnNumber);

//
// ========== Import Operations ==========
//

// Gets the external kind of the specified import.
WASM_API BinaryenExternalKind BinaryenImportGetKind(BinaryenImportRef import);
// Gets the external module name of the specified import.
WASM_API const char* BinaryenImportGetModule(BinaryenImportRef import);
// Gets the external base name of the specified import.
WASM_API const char* BinaryenImportGetBase(BinaryenImportRef import);
// Gets the internal name of the specified import.
WASM_API const char* BinaryenImportGetName(BinaryenImportRef import);
// Gets the type of the imported global, if referencing a `Global`.
WASM_API BinaryenType BinaryenImportGetGlobalType(BinaryenImportRef import);
// Gets the name of the function type of the imported function, if referencing a `Function`.
WASM_API const char* BinaryenImportGetFunctionType(BinaryenImportRef import);

//
// ========== Export Operations ==========
//

// Gets the external kind of the specified export.
WASM_API BinaryenExternalKind BinaryenExportGetKind(BinaryenExportRef export_);
// Gets the external name of the specified export.
WASM_API const char* BinaryenExportGetName(BinaryenExportRef export_);
// Gets the internal name of the specified export.
WASM_API const char* BinaryenExportGetValue(BinaryenExportRef export_);

//
// ========== CFG / Relooper ==========
//
// General usage is (1) create a relooper, (2) create blocks, (3) add
// branches between them, (4) render the output.
//
// See Relooper.h for more details

typedef void* RelooperRef;
typedef void* RelooperBlockRef;

// Create a relooper instance
WASM_API RelooperRef RelooperCreate(void);

// Create a basic block that ends with nothing, or with some simple branching
WASM_API RelooperBlockRef RelooperAddBlock(RelooperRef relooper, BinaryenExpressionRef code);

// Create a branch to another basic block
// The branch can have code on it, that is executed as the branch happens. this is useful for phis. otherwise, code can be NULL
WASM_API void RelooperAddBranch(RelooperBlockRef from, RelooperBlockRef to, BinaryenExpressionRef condition, BinaryenExpressionRef code);

// Create a basic block that ends a switch on a condition
WASM_API RelooperBlockRef RelooperAddBlockWithSwitch(RelooperRef relooper, BinaryenExpressionRef code, BinaryenExpressionRef condition);

// Create a switch-style branch to another basic block. The block's switch table will have these indexes going to that target
WASM_API void RelooperAddBranchForSwitch(RelooperBlockRef from, RelooperBlockRef to, BinaryenIndex* indexes, BinaryenIndex numIndexes, BinaryenExpressionRef code);

// Generate structed wasm control flow from the CFG of blocks and branches that were created
// on this relooper instance. This returns the rendered output, and also disposes of the
// relooper and its blocks and branches, as they are no longer needed.
//   @param labelHelper To render irreducible control flow, we may need a helper variable to
//                      guide us to the right target label. This value should be an index of
//                      an i32 local variable that is free for us to use.
WASM_API BinaryenExpressionRef RelooperRenderAndDispose(RelooperRef relooper, RelooperBlockRef entry, BinaryenIndex labelHelper, BinaryenModuleRef module);

//
// ========= Other APIs =========
//

// Sets whether API tracing is on or off. It is off by default. When on, each call
// to an API method will print out C code equivalent to it, which is useful for
// auto-generating standalone testcases from projects using the API.
// When calling this to turn on tracing, the prelude of the full program is printed,
// and when calling it to turn it off, the ending of the program is printed, giving
// you the full compilable testcase.
// TODO: compile-time option to enable/disable this feature entirely at build time?
WASM_API void BinaryenSetAPITracing(int on);

//
// ========= Utilities =========
//

// Note that this function has been added because there is no better alternative
// currently and is scheduled for removal once there is one. It takes the same set
// of parameters as BinaryenAddFunctionType but instead of adding a new function
// signature, it returns a pointer to the existing signature or NULL if there is no
// such signature yet.
WASM_API BinaryenFunctionTypeRef BinaryenGetFunctionTypeBySignature(BinaryenModuleRef module, BinaryenType result, BinaryenType* paramTypes, BinaryenIndex numParams);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // wasm_binaryen_c_h
