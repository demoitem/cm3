MODULE TestMatchWavelet;

IMPORT LongRealBasic AS R;
IMPORT LongRealTrans AS RT;
IMPORT Integer32IntegerPower AS IIntPow;

IMPORT LongRealVectorFast AS V;
IMPORT LongRealVectorSupport AS VS;
IMPORT LongRealVectorTrans AS VT;

IMPORT LongRealMatrixFast AS M;
IMPORT LongRealMatrixLapack AS LA;

IMPORT LongRealSignal AS S;
IMPORT LongRealSignalIntegerPower AS SIntPow;

IMPORT LongRealRefinableFunc AS Refn;
IMPORT LongRealBSplineWavelet AS BSpl;

IMPORT LongRealFmtLex AS RF;
IMPORT LongRealVectorFmtLex AS VF;
IMPORT LongRealMatrixFmtLex AS MF;
IMPORT LongRealSignalFmtLex AS SF;
IMPORT LongRealWaveletPlot AS WP;
IMPORT PLPlot AS PL;
IMPORT IO, Fmt, Wr, Thread;
IMPORT NADefinitions AS NA;

PROCEDURE MatchPattern (target                               : S.T;
                        levels, smooth, vanishing, translates: CARDINAL)
  RAISES {BSpl.DifferentParity} =
  (*The degree of freedom, i.e.  the number of parameters to minimize for,
     is 2*translates*)
  <*FATAL NA.Error, Thread.Alerted, Wr.Failure*>
  VAR
    hdual := BSpl.GeneratorMask(smooth);
    gdual := BSpl.WaveletMask(smooth, vanishing);
    vancore := SIntPow.MulPower(
                 hdual, NEW(S.T).fromArray(
                          ARRAY OF R.T{1.0D0, 0.0D0, -1.0D0}, -2),
                 vanishing);
    phivan := Refn.Refine(vancore.scale(RT.SqRtTwo), hdual, levels);
    psi    := Refn.Refine(gdual.scale(R.One / RT.SqRtTwo), hdual, levels);

    unit   := IIntPow.Power(2, levels);
    twonit := 2 * unit;
    first  := MIN(psi.getFirst(), phivan.getFirst() - twonit * translates);
    last := MAX(
              psi.getLast(), phivan.getLast() + twonit * (translates - 1));
    size := last - first + 1;

    grid     := R.One / FLOAT(unit, R.T);
    abscissa := V.ArithSeq(size, FLOAT(first, R.T) * grid, grid);

    targetvec := V.New(size);
    basis     := M.New(2 * translates + 1, size);

    coef: LA.LS;

  BEGIN
    PL.Init();
    PL.SetEnvironment(
      abscissa[FIRST(abscissa^)], abscissa[LAST(abscissa^)], -1.5D0 * grid,
      1.5D0 * grid);

    psi.clipToArray(first, basis[LAST(basis^)]);
    PL.SetColor0(1);
    PL.PlotLines(abscissa^, basis[LAST(basis^)]);
    PL.SetColor0(2);
    FOR j := -translates TO translates - 1 DO
      phivan.clipToArray(first - twonit * j, basis[j + translates]);
      PL.PlotLines(abscissa^, basis[j + translates]);
    END;

    target.clipToArray(first, targetvec^);
    PL.SetColor0(3);
    PL.PlotLines(abscissa^, targetvec^);

    coef := LA.LeastSquaresGen(
              basis, ARRAY OF V.T{targetvec},
              flags := LA.LSGenFlagSet{LA.LSGenFlag.transposed})[0];

    IO.Put(Fmt.FN("translates %s, size %s, residuum %s, %s\n",
                  ARRAY OF
                    TEXT{Fmt.Int(translates), Fmt.Int(size),
                         RF.Fmt(coef.res), VF.Fmt(coef.x)}));

    PL.SetColor0(4);
    PL.PlotLines(abscissa^, M.MulTV(basis, coef.x)^);

    PL.Exit();
  END MatchPattern;


PROCEDURE ComputeRho (READONLY y: ARRAY [0 .. 2] OF R.T): R.T =
  <*FATAL NA.Error*>
  VAR
    p1  := VS.Sum(y);
    p2  := VS.Inner(y, y);
    p12 := p1 * p1;
    dif := 3.0D0 * p2 - p12;
  BEGIN
    RETURN dif * dif + 2.0D0 * p12 * p12;
  END ComputeRho;

PROCEDURE ComputeDRho (READONLY y: ARRAY [0 .. 2] OF R.T): V.T =
  <*FATAL NA.Error*>
  VAR
    p1  := VS.Sum(y);
    p2  := VS.Inner(y, y);
    p12 := p1 * p1;
    p1d := 12.0D0 * (p1 * (p12 - p2));
    p2d := 6.0D0 * (3.0D0 * p2 - p12)
             * 2.0D0 (*because p2' contains 2a'a and so on*);
    z := V.New(NUMBER(y));
  BEGIN
    FOR i := 0 TO LAST(y) DO z[i] := p1d + y[i] * p2d; END;
    RETURN z;
  END ComputeDRho;

PROCEDURE ComputeDDRho (READONLY y: ARRAY [0 .. 2] OF R.T): M.T =
  <*FATAL NA.Error*>
  (*derived with mathematica*)
  VAR
    p1 := VS.Sum(y);
    p2 := VS.Inner(y, y);

    z := M.New(NUMBER(y), NUMBER(y));
  BEGIN
    FOR i := 0 TO LAST(y) DO
      FOR j := i TO LAST(y) DO
        z[i, j] := (p1 + y[i] - y[j]) * (p1 - y[i] + y[j]);
        z[j, i] := z[i, j];
      END;
      z[i, i] := z[i, i] + (3.0D0 * y[i] - 2.0D0 * p1) * y[i] + p2;
    END;
    RETURN M.Scale(z, 24.0D0);
  END ComputeDDRho;

PROCEDURE TestRho (x: V.T) =
  VAR
    dx0 := V.FromArray(ARRAY OF R.T{1.0D-8, 0.0D0, 0.0D0});
    dx1 := V.FromArray(ARRAY OF R.T{0.0D0, 1.0D-8, 0.0D0});
    dx2 := V.FromArray(ARRAY OF R.T{0.0D0, 0.0D0, 1.0D-8});
  <*FATAL NA.Error, Thread.Alerted, Wr.Failure*>
  BEGIN
    VAR
      rho     := ComputeRho(x^);
      rho0    := ComputeRho(V.Add(x, dx0)^);
      rho1    := ComputeRho(V.Add(x, dx1)^);
      rho2    := ComputeRho(V.Add(x, dx2)^);
      gradrho := V.Scale(ComputeDRho(x^), 1.0D-8);
    BEGIN
      IO.Put(
        Fmt.FN("rho %s, difrho={%s,%s,%s}, approxdiff=%s\n",
               ARRAY OF
                 TEXT{RF.Fmt(rho), RF.Fmt(rho0 - rho), RF.Fmt(rho1 - rho),
                      RF.Fmt(rho2 - rho), VF.Fmt(gradrho)}));
    END;
    VAR
      gradrho   := ComputeDRho(x^);
      gradrho0  := ComputeDRho(V.Add(x, dx0)^);
      gradrho1  := ComputeDRho(V.Add(x, dx1)^);
      gradrho2  := ComputeDRho(V.Add(x, dx2)^);
      jacobirho := M.Scale(ComputeDDRho(x^), 1.0D-8);
    BEGIN
      IO.Put(
        Fmt.FN("gradrho %s, difgradrho={%s,%s,%s}, approxdiff=%s\n",
               ARRAY OF
                 TEXT{VF.Fmt(gradrho), VF.Fmt(V.Sub(gradrho0, gradrho)),
                      VF.Fmt(V.Sub(gradrho1, gradrho)),
                      VF.Fmt(V.Sub(gradrho2, gradrho)), MF.Fmt(jacobirho)}));
    END;
  END TestRho;

PROCEDURE InverseDRho (x: V.T): V.T RAISES {NA.Error} =
  (*Find the parameter vector y for which DRho(y)=x*)
  CONST tol = 1.0D-14;
  (*VAR y := V.New(3);*)
  VAR y := x;                    (*zero is a bad initial value*)
  BEGIN
    FOR j := 0 TO 100 DO
      VAR ax := ComputeDRho(y^);
      BEGIN
        IF VT.Norm1(V.Sub(ax, x)) <= tol * VT.Norm1(x) THEN RETURN y; END;
        y :=
          V.Add(y, LA.LeastSquaresGen(
                     ComputeDDRho(y^), ARRAY OF V.T{V.Sub(x, ax)})[0].x);
        (*
          IO.Put(Fmt.FN("y %s, DRho(y) %s\n",
                       ARRAY OF TEXT{VF.Fmt(y), VF.Fmt(ComputeDRho(y^))}));
        *)
      END;
    END;
    RAISE NA.Error(NA.Err.not_converging);
  END InverseDRho;

PROCEDURE TestInverseDRho (READONLY x0: ARRAY [0 .. 2] OF R.T) =
  <*FATAL NA.Error, Thread.Alerted, Wr.Failure*>
  VAR
    x := V.FromArray(x0);
    y := InverseDRho(x);
  BEGIN
    IO.Put(
      Fmt.FN("x %s, ComputeDRho(y) %s, y %s\n",
             ARRAY OF TEXT{VF.Fmt(x), VF.Fmt(ComputeDRho(y^)), VF.Fmt(y)}));
  END TestInverseDRho;

TYPE
  Deriv2 = RECORD
             zeroth: R.T;
             first : V.T;
             second: M.T;
           END;

PROCEDURE DeriveDist (normalMat    : M.T;
                      targetCor    : V.T;
                      targetNormSqr: R.T;
                      s            : S.T; ): Deriv2 RAISES {NA.Error} =
  VAR
    normals := M.MulV(normalMat, s.getData());
    dist := V.Inner(s.getData(), V.Sub(normals, V.Scale(targetCor, R.Two)))
              + targetNormSqr;

  BEGIN
    RETURN Deriv2{zeroth := dist, first :=
                  V.Scale(V.Sub(normals, targetCor), R.Two), second :=
                  M.Scale(normalMat, R.Two)};
  END DeriveDist;

PROCEDURE DeriveRho (hdual, gdual0, s: S.T; ): Deriv2 RAISES {NA.Error} =
  VAR
    gdual   := gdual0.superpose(s.upsample(2).convolve(hdual));
    hprimal := gdual.alternate();
    gprimal := hdual.alternate();

    hsums := hprimal.wrapCyclic(3).getData();
    dsums := M.Cyclic(gprimal.translate(2 * s.getFirst()).wrapCyclic(
                        3).getData(), s.getNumber(), -1);
  BEGIN
    (*
    IO.Put(MF.Fmt(dsums) & "\n");
    RETURN polypart;
    *)
    RETURN
      Deriv2{zeroth := ComputeRho(hsums^), first :=
             M.MulV(dsums, ComputeDRho(hsums^)), second :=
             M.Mul(M.Mul(dsums, ComputeDDRho(hsums^)), M.Transpose(dsums))};
  END DeriveRho;

PROCEDURE AddDerv (READONLY x, y: Deriv2): Deriv2 RAISES {NA.Error} =
  BEGIN
    RETURN
      Deriv2{zeroth := x.zeroth + y.zeroth, first :=
             V.Add(x.first, y.first), second := M.Add(x.second, y.second)};
  END AddDerv;

PROCEDURE ScaleDerv (READONLY x: Deriv2; y: R.T): Deriv2 =
  BEGIN
    RETURN Deriv2{zeroth := x.zeroth * y, first := V.Scale(x.first, y),
                  second := M.Scale(x.second, y)};
  END ScaleDerv;

PROCEDURE PutDervDif (READONLY der   : Deriv2;
                      READONLY derArr: ARRAY OF Deriv2;
                               delta : R.T              ) =
  BEGIN
    (*compare first derivative (gradient) with the first difference*)
    IO.Put(VF.Fmt(V.Scale(der.first, delta)) & "\n");
    FOR j := 0 TO LAST(derArr) DO
      IO.Put(Fmt.FN("der[%s] %s, %s\n",
                    ARRAY OF
                      TEXT{Fmt.Int(j (* + y.getFirst()*)),
                           RF.Fmt(derArr[j].zeroth - der.zeroth),
                           RF.Fmt(der.first[j] * delta)}));
    END;

    (*compare second derivative with the first difference of the first
       derivative*)
    FOR j := 0 TO LAST(derArr) DO
      IO.Put(
        Fmt.FN("der[%s] %s, %s\n",
               ARRAY OF
                 TEXT{Fmt.Int(j (*+ y.getFirst()*)),
                      VF.Fmt(V.Sub(derArr[j].first, der.first)),
                      VF.Fmt(V.Scale(M.GetRow(der.second, j), delta))}));
    END;
  END PutDervDif;


(*extend derivatives by information that are contributed by optimizing the
   amplitude of the target*)
PROCEDURE ExtendDervTarget (READONLY x        : Deriv2;
                                     lift     : V.T;
                                     targetAmp: R.T;
                                     target   : V.T;
                                     targetCor: V.T;
                                     wavelet0 : V.T     ): Deriv2 =
  BEGIN
    RETURN
      Deriv2{
        zeroth := x.zeroth + R.Two * targetAmp * V.Inner(targetCor, lift)
                    + targetAmp * targetAmp * V.Inner(target, target)
                    - R.Two * targetAmp * V.Inner(wavelet0, target),
        first :=
        V.FromVectorArray(
          ARRAY OF
            V.T{
            V.Add(x.first, V.Scale(targetCor, targetAmp * R.Two)),
            V.FromScalar(R.Two * (V.Inner(lift, targetCor)
                                    + targetAmp * V.Inner(target, target)
                                    - V.Inner(wavelet0, target)))}),
        second :=
        M.FromMatrixArray(
          ARRAY [0 .. 1], [0 .. 1] OF
            M.T{
            ARRAY OF
              M.T{x.second, M.ColumnFromArray(V.Scale(targetCor, R.Two)^)},
            ARRAY OF
              M.T{M.RowFromArray(V.Scale(targetCor, R.Two)^),
                  M.FromScalar(V.Inner(target, target) * R.Two)}})};
  END ExtendDervTarget;

PROCEDURE TranslatesBasis (generatorvan: S.T;
                           first       : INTEGER;
                           twonit, size: CARDINAL;
                           translates  : CARDINAL  ): M.T =
  VAR basis := M.New(2 * translates, size);
  BEGIN
    FOR j := -translates TO translates - 1 DO
      generatorvan.clipToArray(first - twonit * j, basis[j + translates]);
      (*
            targetCor[j + translates] :=
              VS.Inner(basis[j + translates], targetVec^);
      *)
    END;
    RETURN basis;
  END TranslatesBasis;

TYPE
  MatchCoef = RECORD
                lift     : S.T;
                targetAmp: R.T;  (*coefficient of the target function*)
              END;

PROCEDURE MatchPatternSmooth (wavelet               : S.T;
                              hdual, gdual, hdualvan: S.T;
                              levels, translates    : CARDINAL;
                              smoothWeight          : R.T;      ):
  MatchCoef RAISES {NA.Error} =

  <*UNUSED*>
  PROCEDURE CheckDerivatives () =
    CONST
      delta = 1.0D-8;
      cf    = 0.7D0;
    VAR
      (*this array length can only be used if a total of 6 translates is
         considered*)
      y := NEW(S.T).fromArray(
             ARRAY OF R.T{0.2D0, -0.3D0, 0.0D0, -0.1D0, 0.0D0, 0.4D0}, -3);
      der := AddDerv(
               DeriveDist(normalMat, waveletCor, waveletNormSqr, y),
               ScaleDerv(DeriveRho(hdualvan, gdual, y), smoothWeight));
      derArr := NEW(REF ARRAY OF Deriv2, NUMBER(der.first^));
      extder := ExtendDervTarget(
                  der, y.getData(), cf, targetVec, targetCor, waveletVec);
      extderArr := NEW(REF ARRAY OF Deriv2, NUMBER(extder.first^));

    BEGIN
      FOR j := 0 TO LAST(derArr^) DO
        VAR
          yp := y.superpose(NEW(S.T).fromArray(
                              ARRAY OF R.T{delta}, j + y.getFirst()));
        BEGIN
          derArr[j] :=
            AddDerv(
              DeriveDist(normalMat, waveletCor, waveletNormSqr, yp),
              ScaleDerv(DeriveRho(hdualvan, gdual, yp), smoothWeight));
          extderArr[j] :=
            ExtendDervTarget(derArr[j], yp.getData(), cf, targetVec,
                             targetCor, waveletVec);
        END;
      END;
      extderArr[LAST(extderArr^)] :=
        ExtendDervTarget(
          der, y.getData(), cf + delta, targetVec, targetCor, waveletVec);

      PutDervDif(der, derArr^, delta);
      PutDervDif(extder, extderArr^, delta);
    END CheckDerivatives;

  VAR
    generatorvan := Refn.Refine(hdualvan.scale(RT.SqRtTwo), hdual, levels);
    target := Refn.Refine(gdual.scale(R.One / RT.SqRtTwo), hdual, levels);

    unit   := IIntPow.Power(2, levels);
    twonit := 2 * unit;
    first := MIN(target.getFirst(),
                 generatorvan.getFirst() - twonit * translates);
    last := MAX(target.getLast(),
                generatorvan.getLast() + twonit * (translates - 1));
    size := last - first + 1;

    waveletVec     := wavelet.clipToVector(first, size);
    waveletNormSqr := V.Inner(waveletVec, waveletVec);
    targetVec      := target.clipToVector(first, size);
    targetNormSqr  := V.Inner(targetVec, targetVec);
    (* the target vector might have been clipped, thus
       V.Inner(target.getData(),target.getData()) may be different *)

    basis := TranslatesBasis(generatorvan, first, twonit, size, translates);
    normalMat  := M.MMA(basis);
    targetCor  := M.MulV(basis, targetVec);
    waveletCor := M.MulV(basis, waveletVec);

  BEGIN
    (*
    CheckDerivatives();
    *)

    CONST tol = 1.0D-14;
    VAR
      yfirst := -translates;
      y      := NEW(S.T).fromArray(V.ArithSeq(2 * translates)^, yfirst);
      cf     := R.One;
    (*
      y := V.FromVectorArray(
             ARRAY OF V.T{V.ArithSeq(2 * translates), V.FromScalar(R.One)});
    *)
    BEGIN
      FOR j := 0 TO 100 DO
        VAR
          der := AddDerv(
                   DeriveDist(normalMat, waveletCor, waveletNormSqr, y),
                   ScaleDerv(DeriveRho(hdualvan, gdual, y), smoothWeight));
          extder := ExtendDervTarget(der, y.getData(), cf, targetVec,
                                     targetCor, waveletVec);

        (* targetdiff := V.Sub(targetVec, M.MulTV(basis, y.getData()));
           targetdist := V.Inner(targetdiff, targetdiff); *)
        BEGIN
          IF VT.Norm1(extder.first) <= tol * RT.Abs(extder.zeroth) THEN
            RETURN MatchCoef{y, cf};
          END;
          (*
            IO.Put(Fmt.FN("derivatives %s, %s, %s\n",
                          ARRAY OF
                            TEXT{RF.Fmt(der.zeroth), VF.Fmt(der.first),
                                 MF.Fmt(der.second)}));
          *)
          VAR
            vec := LA.LeastSquaresGen(
                     extder.second, ARRAY OF V.T{V.Neg(extder.first)})[0].x;
          BEGIN
            cf := cf + vec[LAST(vec^)];
            y := y.superpose(NEW(S.T).fromArray(SUBARRAY(vec^, FIRST(vec^),
                                                         NUMBER(vec^) - 1),
                                                yfirst));
            IO.Put(
              Fmt.FN("y %s, cf %s\n", ARRAY OF TEXT{SF.Fmt(y), RF.Fmt(cf)}));
          END;
        END;
      END;
      RAISE NA.Error(NA.Err.not_converging);
    END;
  END MatchPatternSmooth;

PROCEDURE TestMatchPatternSmooth (target: S.T;
                                  levels, smooth, vanishing, translates: CARDINAL;
                                  smoothWeight: R.T)
  RAISES {BSpl.DifferentParity} =
  <*FATAL NA.Error, Thread.Alerted, Wr.Failure*>
  CONST
    ymin = -1.5D0;
    ymax = 1.5D0;
  VAR
    hdual  := BSpl.GeneratorMask(smooth);
    gdual0 := BSpl.WaveletMask(smooth, vanishing);
    hdualvan := SIntPow.MulPower(
                  hdual, NEW(S.T).fromArray(
                           ARRAY OF R.T{1.0D0, 0.0D0, -1.0D0}, -2),
                  vanishing);
    mc := MatchPatternSmooth(target, hdual, gdual0, hdualvan, levels,
                             translates, smoothWeight);
    s := SIntPow.MulPower(
           mc.lift, NEW(S.T).fromArray(ARRAY OF R.T{1.0D0, -1.0D0}, -1),
           vanishing);
    gdual := gdual0.scale(1.0D0 / RT.SqRtTwo).superpose(
               s.upsample(2).convolve(
                 hdual.scale(RT.SqRtTwo / mc.targetAmp)));

    unit          := IIntPow.Power(2, levels);
    twopow        := FLOAT(unit, R.T);
    grid          := R.One / twopow;
    psidual0      := Refn.Refine(gdual0, hdual, levels).scale(twopow);
    psidual       := Refn.Refine(gdual, hdual, levels).scale(twopow);
    leftpsidual0  := FLOAT(psidual0.getFirst(), R.T) * grid;
    rightpsidual0 := FLOAT(psidual0.getLast(), R.T) * grid;
    leftpsidual   := FLOAT(psidual.getFirst(), R.T) * grid;
    rightpsidual  := FLOAT(psidual.getLast(), R.T) * grid;
    lefttarget    := FLOAT(target.getFirst(), R.T) * grid;
    righttarget   := FLOAT(target.getLast(), R.T) * grid;

  BEGIN
    IO.Put(
      Fmt.FN(
        "optimal lift %s,\ncyclic wrap of gdual %s\n",
        ARRAY OF TEXT{SF.Fmt(s), SF.Fmt(gdual.alternate().wrapCyclic(3))}));
    PL.Init();
    CASE 1 OF
    | 0 => WP.PlotWavelets(hdual, gdual, levels);
    | 1 =>
        PL.SetEnvironment(MIN(lefttarget, leftpsidual),
                          MAX(righttarget, rightpsidual), ymin, ymax);
        PL.PlotLines(V.ArithSeq(target.getNumber(), lefttarget, grid)^,
                     target.scale(twopow / mc.targetAmp).getData()^);
        PL.SetColor0(2);
        PL.PlotLines(V.ArithSeq(psidual0.getNumber(), leftpsidual0, grid)^,
                     psidual0.getData()^);
        PL.SetColor0(3);
        PL.PlotLines(V.ArithSeq(psidual.getNumber(), leftpsidual, grid)^,
                     psidual.getData()^);
    ELSE
      <*ASSERT FALSE*>
    END;
    PL.Exit();
  END TestMatchPatternSmooth;

PROCEDURE Test () =
  <*FATAL BSpl.DifferentParity*>
  BEGIN
    CASE 3 OF
    | 0 =>
        MatchPattern(
          Refn.Refine(S.One, BSpl.GeneratorMask(4), 7).translate(-50), 6,
          4, 0, 5);
    | 1 =>
        MatchPattern(
          Refn.Refine(S.One, BSpl.GeneratorMask(1), 7).translate(10), 6, 4,
          2, 5);
      (*
      0.515299272019265, 1.09394334049039, 2.37421841031068, 0.167484610002095, -1.6496475444949, -4.50118081829745, -2.99168284568131, -2.07333727163662, -1.13262489000281, -0.469773425085895
      -0.482838852151025
      *)
    | 2 =>
        MatchPattern(
          NEW(S.T).fromArray(
            V.ArithSeq(512, -0.01D0, 0.02D0 / 512.0D0)^, -256), 6, 4, 2, 5);
      (*
      y Signal[-5..4]{0.0591062446152401, 0.0903181993065596, 0.285545159210211, -0.136564892299842, 1.27478030776691, -0.587901480689649, -0.234139988943924, -0.21826373576695, -0.103008677224905, -0.0482341791772408}, cf -0.096377134505568
      *)
    | 3 =>
        TestMatchPatternSmooth(
          NEW(S.T).fromArray(
            V.ArithSeq(512, -0.01D0, 0.02D0 / 512.0D0)^, -256), 6, 4, 2, 5,
          0.0D0);
    | 4 =>
        TestMatchPatternSmooth(
          NEW(S.T).fromArray(
            V.ArithSeq(2048, -1.0D0, 2.0D0 / 2048.0D0)^, -1024), 6, 3, 9,
          1, 100.0D0);
    | 5 =>
        TestRho(V.FromArray(ARRAY OF R.T{0.9D0, 0.7D0, -0.6D0}));
        TestRho(V.FromArray(ARRAY OF R.T{1.0D0, 1.0D0, 1.0D0}));
    | 6 =>
        TestInverseDRho(ARRAY OF R.T{0.9D0, 0.7D0, -0.6D0});
        TestInverseDRho(ARRAY OF R.T{1.0D0, 1.0D0, 0.1D0});
        TestInverseDRho(ARRAY OF R.T{1.0D0, 1.0D0, 1.0D0});
    ELSE
      <*ASSERT FALSE*>
    END;
  END Test;

BEGIN
END TestMatchWavelet.
