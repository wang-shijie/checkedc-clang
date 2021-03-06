//===-- CanonBounds.h - compare and canonicalize bounds exprs --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the interface for comparing and canonicalizing
//  bounds expressions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_CANON_BOUNDS_H
#define LLVM_CLANG_CANON_BOUNDS_H

#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Type.h"
#include "clang/Basic/AttrKinds.h"
#include "clang/Basic/LLVM.h"
#include "clang/Basic/OpenMPKinds.h"
#include "clang/Basic/Sanitizers.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/VersionTuple.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cassert>

namespace clang {
  class ASTContext;
  class Expr;
  class VarDecl;

  // Abstract base class that provides information what variables
  // currently are equal to each other.
  class EqualityRelation {
  public:
     virtual const VarDecl *getRepresentative(const VarDecl *V) = 0;
  };

  class Lexicographic {
  public:
    enum class Result {
      LessThan,
      Equal,
      GreaterThan
    };

  private:
    ASTContext &Context;
    EqualityRelation *EqualVars;
    bool Trace;

    template <typename T>
    Lexicographic::Result Compare(const Expr *Raw1, const Expr *Raw2) {
      const T *E1 = dyn_cast<T>(Raw1);
      const T *E2 = dyn_cast<T>(Raw2);
      if (!E1 || !E2) {
        llvm_unreachable("dyn_cast failed");
        return Result::LessThan;
      }
      return Lexicographic::CompareImpl(E1, E2);
    }

    Result CompareInteger(signed I1, signed I2);
    Result CompareInteger(unsigned I1, unsigned I2);
    Result CompareRelativeBoundsClause(const RelativeBoundsClause *RC1,
                                       const RelativeBoundsClause *RC2);
    Result CompareScope(const DeclContext *DC1, const DeclContext *DC2);

    Result CompareImpl(const PredefinedExpr *E1, const PredefinedExpr *E2);
    Result CompareImpl(const DeclRefExpr *E1, const DeclRefExpr *E2);
    Result CompareImpl(const IntegerLiteral *E1, const IntegerLiteral *E2);
    Result CompareImpl(const FloatingLiteral *E1, const FloatingLiteral *E2);
    Result CompareImpl(const StringLiteral *E1, const StringLiteral *E2);
    Result CompareImpl(const CharacterLiteral *E1, const CharacterLiteral *E2);
    Result CompareImpl(const UnaryOperator *E1, const UnaryOperator *E2);
    Result CompareImpl(const OffsetOfExpr *E1, const OffsetOfExpr *E2);
    Result CompareImpl(const UnaryExprOrTypeTraitExpr *E1,
                   const UnaryExprOrTypeTraitExpr *E2);
    Result CompareImpl(const MemberExpr *E1, const MemberExpr *E2);
    Result CompareImpl(const BinaryOperator *E1, const BinaryOperator *E2);
    Result CompareImpl(const CompoundAssignOperator *E1,
                   const CompoundAssignOperator *E2);
    Result CompareImpl(const ImplicitCastExpr *E1, const ImplicitCastExpr *E2);
    Result CompareImpl(const CStyleCastExpr *E1, const CStyleCastExpr *E2);
    Result CompareImpl(const CompoundLiteralExpr *E1,
                   const CompoundLiteralExpr *E2);
    Result CompareImpl(const GenericSelectionExpr *E1,
                   const GenericSelectionExpr *E2);
    Result CompareImpl(const NullaryBoundsExpr *E1,
                       const NullaryBoundsExpr *E2);
    Result CompareImpl(const CountBoundsExpr *E1, const CountBoundsExpr *E2);
    Result CompareImpl(const RangeBoundsExpr *E1, const RangeBoundsExpr *E2);
    Result CompareImpl(const InteropTypeExpr *E1, const InteropTypeExpr *E2);
    Result CompareImpl(const PositionalParameterExpr *E1,
                   const PositionalParameterExpr *E2);
    Result CompareImpl(const BoundsCastExpr *E1, const BoundsCastExpr *E2);
    Result CompareImpl(const AtomicExpr *E1, const AtomicExpr *E2);
    Result CompareImpl(const BlockExpr *E1, const BlockExpr *E2);


  public:
    Lexicographic(ASTContext &Ctx, EqualityRelation *EV);

    /// \brief Lexicographic comparison of expressions that can occur in
    /// bounds expressions.
    Result CompareExpr(const Expr *E1, const Expr *E2);

    /// \brief Compare declarations that may be used by expressions or
    /// or types.
    Result CompareDecl(const NamedDecl *D1, const NamedDecl *D2);
    Result CompareType(QualType T1, QualType T2);
  };
}  // end namespace clang

#endif
