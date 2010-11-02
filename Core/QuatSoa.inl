//----------------------------------------------------------------------------------------------------------------------
// QuatSoa.inl
//
// Copyright (C) 2010 WhiteMoon Dreams, Inc.
// All Rights Reserved
//----------------------------------------------------------------------------------------------------------------------

namespace Lunar
{
    /// Constructor.
    ///
    /// This creates a quaternion with all components uninitialized.  Values should be assigned before use.
    QuatSoa::QuatSoa()
    {
    }

    /// Constructor.
    ///
    /// @param[in] rX  X components.
    /// @param[in] rY  Y components.
    /// @param[in] rZ  Z components.
    /// @param[in] rW  W components.
    QuatSoa::QuatSoa( const SimdVector& rX, const SimdVector& rY, const SimdVector& rZ, const SimdVector& rW )
        : m_x( rX )
        , m_y( rY )
        , m_z( rZ )
        , m_w( rW )
    {
    }

    /// Constructor.
    ///
    /// This will fully load the SIMD vectors for each component from the given addresses.
    ///
    /// @param[in] pX  X components (must be SIMD aligned).
    /// @param[in] pY  Y components (must be SIMD aligned).
    /// @param[in] pZ  Z components (must be SIMD aligned).
    /// @param[in] pW  W components (must be SIMD aligned).
    QuatSoa::QuatSoa( const float32_t* pX, const float32_t* pY, const float32_t* pZ, const float32_t* pW )
    {
        Load( pX, pY, pZ, pW );
    }

    /// Constructor.
    ///
    /// This will initialize this quaternion by splatting each component of the given quaternion across a full SIMD
    /// vector.
    ///
    /// @param[in] rQuat  Quaternion from which to initialize this quaternion.
    QuatSoa::QuatSoa( const Quat& rQuat )
    {
        Splat( rQuat );
    }

    /// Fully load the SIMD vectors for each quaternion component.
    ///
    /// @param[in] pX  X components (must be SIMD aligned).
    /// @param[in] pY  Y components (must be SIMD aligned).
    /// @param[in] pZ  Z components (must be SIMD aligned).
    /// @param[in] pW  W components (must be SIMD aligned).
    void QuatSoa::Load( const float32_t* pX, const float32_t* pY, const float32_t* pZ, const float32_t* pW )
    {
        m_x = Simd::LoadAligned( pX );
        m_y = Simd::LoadAligned( pY );
        m_z = Simd::LoadAligned( pZ );
        m_w = Simd::LoadAligned( pW );
    }

    /// Load 4 single-precision floating-point values for each quaternion component, splatting the values to fill.
    ///
    /// If the current platform SIMD vector format is only large enough to contain 4 floats, this will have the exact
    /// same effect as Load().
    ///
    /// @param[in] pX  X components (must be aligned to a 16-byte boundary).
    /// @param[in] pY  Y components (must be aligned to a 16-byte boundary).
    /// @param[in] pZ  Z components (must be aligned to a 16-byte boundary).
    /// @param[in] pW  W components (must be aligned to a 16-byte boundary).
    void QuatSoa::Load4Splat( const float32_t* pX, const float32_t* pY, const float32_t* pZ, const float32_t* pW )
    {
        m_x = Simd::LoadSplat128( pX );
        m_y = Simd::LoadSplat128( pY );
        m_z = Simd::LoadSplat128( pZ );
        m_w = Simd::LoadSplat128( pW );
    }

    /// Load 1 single-precision floating-point value for each quaternion component, splatting the value to fill.
    ///
    /// @param[in] pX  X components (must be aligned to a 4-byte boundary).
    /// @param[in] pY  Y components (must be aligned to a 4-byte boundary).
    /// @param[in] pZ  Z components (must be aligned to a 4-byte boundary).
    /// @param[in] pW  W components (must be aligned to a 4-byte boundary).
    void QuatSoa::Load1Splat( const float32_t* pX, const float32_t* pY, const float32_t* pZ, const float32_t* pW )
    {
        m_x = Simd::LoadSplat32( pX );
        m_y = Simd::LoadSplat32( pY );
        m_z = Simd::LoadSplat32( pZ );
        m_w = Simd::LoadSplat32( pW );
    }

    /// Fully store the SIMD vectors from each quaternion component into memory.
    ///
    /// @param[out] pX  X components (must be SIMD aligned).
    /// @param[out] pY  Y components (must be SIMD aligned).
    /// @param[out] pZ  Z components (must be SIMD aligned).
    /// @param[out] pW  W components (must be SIMD aligned).
    void QuatSoa::Store( float32_t* pX, float32_t* pY, float32_t* pZ, float32_t* pW ) const
    {
        Simd::StoreAligned( pX, m_x );
        Simd::StoreAligned( pY, m_y );
        Simd::StoreAligned( pZ, m_z );
        Simd::StoreAligned( pW, m_w );
    }

    /// Store the lowest 4 single-precision floating-point values from each quaternion component into memory.
    ///
    /// If the current platform SIMD vector format is only large enough to contain 4 floats, this will have the exact
    /// same effect as Store().
    ///
    /// @param[out] pX  X components (must be aligned to a 16-byte boundary).
    /// @param[out] pY  Y components (must be aligned to a 16-byte boundary).
    /// @param[out] pZ  Z components (must be aligned to a 16-byte boundary).
    /// @param[out] pW  W components (must be aligned to a 16-byte boundary).
    void QuatSoa::Store4( float32_t* pX, float32_t* pY, float32_t* pZ, float32_t* pW ) const
    {
        Simd::Store128( pX, m_x );
        Simd::Store128( pY, m_y );
        Simd::Store128( pZ, m_z );
        Simd::Store128( pW, m_w );
    }

    /// Store the lowest single-precision floating-point value from each quaternion component into memory.
    ///
    /// @param[out] pX  X components (must be aligned to a 4-byte boundary).
    /// @param[out] pY  Y components (must be aligned to a 4-byte boundary).
    /// @param[out] pZ  Z components (must be aligned to a 4-byte boundary).
    /// @param[out] pW  W components (must be aligned to a 4-byte boundary).
    void QuatSoa::Store1( float32_t* pX, float32_t* pY, float32_t* pZ, float32_t* pW ) const
    {
        Simd::Store32( pX, m_x );
        Simd::Store32( pY, m_y );
        Simd::Store32( pZ, m_z );
        Simd::Store32( pW, m_w );
    }

    /// Perform a component-wise addition of this quaternion and another quaternion.
    ///
    /// @param[in] rQuat  Quaternion to add.
    ///
    /// @return  Quaternion containing the sums of each component.
    QuatSoa QuatSoa::Add( const QuatSoa& rQuat ) const
    {
        return QuatSoa(
            Simd::AddF32( m_x, rQuat.m_x ),
            Simd::AddF32( m_y, rQuat.m_y ),
            Simd::AddF32( m_z, rQuat.m_z ),
            Simd::AddF32( m_w, rQuat.m_w ) );
    }

    /// Perform a component-wise subtraction of a quaternion from this quaternion.
    ///
    /// @param[in] rQuat  Quaternion to subtract.
    ///
    /// @return  Quaternion containing the differences of each component.
    QuatSoa QuatSoa::Subtract( const QuatSoa& rQuat ) const
    {
        return QuatSoa(
            Simd::SubtractF32( m_x, rQuat.m_x ),
            Simd::SubtractF32( m_y, rQuat.m_y ),
            Simd::SubtractF32( m_z, rQuat.m_z ),
            Simd::SubtractF32( m_w, rQuat.m_w ) );
    }

    /// Perform a quaternion multiplication between this quaternion and another quaternion.
    ///
    /// @param[in] rQuat  Quaternion with which to multiply.
    ///
    /// @return  Product quaternion.
    QuatSoa QuatSoa::Multiply( const QuatSoa& rQuat ) const
    {
        SimdVector x = Simd::MultiplyF32( m_w, rQuat.m_x );
        SimdVector y = Simd::MultiplyF32( m_w, rQuat.m_y );
        SimdVector z = Simd::MultiplyF32( m_w, rQuat.m_z );
        SimdVector w = Simd::MultiplyF32( m_w, rQuat.m_w );

        x = Simd::MultiplyAddF32( m_x, rQuat.m_w, x );
        y = Simd::MultiplyAddF32( m_y, rQuat.m_w, y );
        z = Simd::MultiplyAddF32( m_z, rQuat.m_w, z );
        w = Simd::MultiplySubtractReverseF32( m_x, rQuat.m_x, w );

        x = Simd::MultiplyAddF32( m_z, rQuat.m_y, x );
        y = Simd::MultiplyAddF32( m_x, rQuat.m_z, y );
        z = Simd::MultiplyAddF32( m_y, rQuat.m_x, z );
        w = Simd::MultiplySubtractReverseF32( m_y, rQuat.m_y, w );

        x = Simd::MultiplySubtractReverseF32( m_y, rQuat.m_z, x );
        y = Simd::MultiplySubtractReverseF32( m_z, rQuat.m_x, y );
        z = Simd::MultiplySubtractReverseF32( m_x, rQuat.m_y, z );
        w = Simd::MultiplySubtractReverseF32( m_z, rQuat.m_z, w );

        return QuatSoa( x, y, z, w );
    }

    /// Perform a component-wise multiplication of this quaternion and another quaternion.
    ///
    /// @param[in] rQuat  Quaternion with which to multiply.
    ///
    /// @return  Quaternion containing the products of each component.
    QuatSoa QuatSoa::MultiplyComponents( const QuatSoa& rQuat ) const
    {
        return QuatSoa(
            Simd::MultiplyF32( m_x, rQuat.m_x ),
            Simd::MultiplyF32( m_y, rQuat.m_y ),
            Simd::MultiplyF32( m_z, rQuat.m_z ),
            Simd::MultiplyF32( m_w, rQuat.m_w ) );
    }

    /// Perform a component-wise division of this quaternion and another quaternion.
    ///
    /// @param[in] rQuat  Quaternion with which to divide.
    ///
    /// @return  Quaternion containing the quotients of each component.
    QuatSoa QuatSoa::DivideComponents( const QuatSoa& rQuat ) const
    {
        return QuatSoa(
            Simd::DivideF32( m_x, rQuat.m_x ),
            Simd::DivideF32( m_y, rQuat.m_y ),
            Simd::DivideF32( m_z, rQuat.m_z ),
            Simd::DivideF32( m_w, rQuat.m_w ) );
    }

    /// Set this quaternion to the component-wise sum of two quaternions.
    ///
    /// @param[in] rQuat0  First quaternion.
    /// @param[in] rQuat1  Second quaternion.
    void QuatSoa::AddSet( const QuatSoa& rQuat0, const QuatSoa& rQuat1 )
    {
        m_x = Simd::AddF32( rQuat0.m_x, rQuat1.m_x );
        m_y = Simd::AddF32( rQuat0.m_y, rQuat1.m_y );
        m_z = Simd::AddF32( rQuat0.m_z, rQuat1.m_z );
        m_w = Simd::AddF32( rQuat0.m_w, rQuat1.m_w );
    }

    /// Set this quaternion to the component-wise difference of two quaternions.
    ///
    /// @param[in] rQuat0  First quaternion.
    /// @param[in] rQuat1  Second quaternion.
    void QuatSoa::SubtractSet( const QuatSoa& rQuat0, const QuatSoa& rQuat1 )
    {
        m_x = Simd::SubtractF32( rQuat0.m_x, rQuat1.m_x );
        m_y = Simd::SubtractF32( rQuat0.m_y, rQuat1.m_y );
        m_z = Simd::SubtractF32( rQuat0.m_z, rQuat1.m_z );
        m_w = Simd::SubtractF32( rQuat0.m_w, rQuat1.m_w );
    }

    /// Set this quaternion to the product of two quaternions.
    ///
    /// @param[in] rQuat0  First quaternion.
    /// @param[in] rQuat1  Second quaternion.
    void QuatSoa::MultiplySet( const QuatSoa& rQuat0, const QuatSoa& rQuat1 )
    {
        SimdVector x = Simd::MultiplyF32( rQuat0.m_w, rQuat1.m_x );
        SimdVector y = Simd::MultiplyF32( rQuat0.m_w, rQuat1.m_y );
        SimdVector z = Simd::MultiplyF32( rQuat0.m_w, rQuat1.m_z );
        SimdVector w = Simd::MultiplyF32( rQuat0.m_w, rQuat1.m_w );

        x = Simd::MultiplyAddF32( rQuat0.m_x, rQuat1.m_w, x );
        y = Simd::MultiplyAddF32( rQuat0.m_y, rQuat1.m_w, y );
        z = Simd::MultiplyAddF32( rQuat0.m_z, rQuat1.m_w, z );
        w = Simd::MultiplySubtractReverseF32( rQuat0.m_x, rQuat1.m_x, w );

        x = Simd::MultiplyAddF32( rQuat0.m_z, rQuat1.m_y, x );
        y = Simd::MultiplyAddF32( rQuat0.m_x, rQuat1.m_z, y );
        z = Simd::MultiplyAddF32( rQuat0.m_y, rQuat1.m_x, z );
        w = Simd::MultiplySubtractReverseF32( rQuat0.m_y, rQuat1.m_y, w );

        x = Simd::MultiplySubtractReverseF32( rQuat0.m_y, rQuat1.m_z, x );
        y = Simd::MultiplySubtractReverseF32( rQuat0.m_z, rQuat1.m_x, y );
        z = Simd::MultiplySubtractReverseF32( rQuat0.m_x, rQuat1.m_y, z );
        w = Simd::MultiplySubtractReverseF32( rQuat0.m_z, rQuat1.m_z, w );

        m_x = x;
        m_y = y;
        m_z = z;
        m_w = w;
    }

    /// Set this quaternion to the component-wise product of two quaternions.
    ///
    /// @param[in] rQuat0  First quaternion.
    /// @param[in] rQuat1  Second quaternion.
    void QuatSoa::MultiplyComponentsSet( const QuatSoa& rQuat0, const QuatSoa& rQuat1 )
    {
        m_x = Simd::MultiplyF32( rQuat0.m_x, rQuat1.m_x );
        m_y = Simd::MultiplyF32( rQuat0.m_y, rQuat1.m_y );
        m_z = Simd::MultiplyF32( rQuat0.m_z, rQuat1.m_z );
        m_w = Simd::MultiplyF32( rQuat0.m_w, rQuat1.m_w );
    }

    /// Set this quaternion to the component-wise quotient of two quaternions.
    ///
    /// @param[in] rQuat0  First quaternion.
    /// @param[in] rQuat1  Second quaternion.
    void QuatSoa::DivideComponentsSet( const QuatSoa& rQuat0, const QuatSoa& rQuat1 )
    {
        m_x = Simd::DivideF32( rQuat0.m_x, rQuat1.m_x );
        m_y = Simd::DivideF32( rQuat0.m_y, rQuat1.m_y );
        m_z = Simd::DivideF32( rQuat0.m_z, rQuat1.m_z );
        m_w = Simd::DivideF32( rQuat0.m_w, rQuat1.m_w );
    }

    /// Get the magnitude of this quaternion.
    ///
    /// @return  Quaternion magnitude.
    SimdVector QuatSoa::GetMagnitude() const
    {
        return Simd::SqrtF32( GetMagnitudeSquared() );
    }

    /// Get the squared magnitude of this quaternion.
    ///
    /// @return  Squared quaternion magnitude.
    SimdVector QuatSoa::GetMagnitudeSquared() const
    {
        SimdVector result = Simd::MultiplyF32( m_x, m_x );
        result = Simd::MultiplyAddF32( m_y, m_y, result );
        result = Simd::MultiplyAddF32( m_z, m_z, result );
        result = Simd::MultiplyAddF32( m_w, m_w, result );

        return result;
    }

    /// Get a normalized copy of this quaternion, with safety threshold checking.
    ///
    /// If the magnitude of this quaternion is below the given epsilon, an identity quaternion will be returned.
    ///
    /// @param[in] rEpsilon  Threshold at which to test for zero-length quaternions.
    ///
    /// @return  Normalized copy of this quaternion.
    ///
    /// @see Normalize()
    QuatSoa QuatSoa::GetNormalized( const SimdVector& rEpsilon ) const
    {
        QuatSoa result = *this;
        result.Normalize( rEpsilon );

        return result;
    }

    /// Normalize this quaternion, with safety threshold checking.
    ///
    /// If the magnitude of this quaternion is below the given epsilon, it will be set to an identity quaternion.
    ///
    /// @param[in] rEpsilon  Threshold at which to test for zero-length quaternions.
    ///
    /// @see GetNormalized()
    void QuatSoa::Normalize( const SimdVector& rEpsilon )
    {
        SimdVector magnitudeSquared = GetMagnitudeSquared();
        SimdVector epsilonSquared = Simd::MultiplyF32( rEpsilon, rEpsilon );

        SimdMask thresholdMask = Simd::GreaterEqualsF32( magnitudeSquared, epsilonSquared );

        SimdVector invMagnitude = Simd::InverseSqrtF32( magnitudeSquared );

        SimdVector normalizedX = Simd::MultiplyF32( m_x, invMagnitude );
        SimdVector normalizedY = Simd::MultiplyF32( m_y, invMagnitude );
        SimdVector normalizedZ = Simd::MultiplyF32( m_z, invMagnitude );
        SimdVector normalizedW = Simd::MultiplyF32( m_w, invMagnitude );

        SimdVector oneVec = Simd::SetSplatF32( 1.0f );

        m_x = Simd::And( normalizedX, thresholdMask );
        m_y = Simd::And( normalizedY, thresholdMask );
        m_z = Simd::And( normalizedZ, thresholdMask );
        m_w = Simd::Select( oneVec, normalizedW, thresholdMask );
    }

    /// Get the inverse of this quaternion.
    ///
    /// @param[out] rQuat  Quaternion inverse.
    ///
    /// @see Invert(), GetConjugate(), SetConjugate()
    void QuatSoa::GetInverse( QuatSoa& rQuat ) const
    {
        SimdVector invMagSquared = Simd::InverseF32( GetMagnitudeSquared() );

        SimdVector signFlip = Simd::SetSplatU32( 0x80000000 );

        SimdVector x = Simd::Xor( m_x, signFlip );
        SimdVector y = Simd::Xor( m_y, signFlip );
        SimdVector z = Simd::Xor( m_z, signFlip );

        rQuat.m_x = Simd::MultiplyF32( x, invMagSquared );
        rQuat.m_y = Simd::MultiplyF32( y, invMagSquared );
        rQuat.m_z = Simd::MultiplyF32( z, invMagSquared );
        rQuat.m_w = Simd::MultiplyF32( m_w, invMagSquared );
    }

    /// Get the inverse of this quaternion.
    ///
    /// @return  Quaternion inverse.
    ///
    /// @see Invert(), GetConjugate(), SetConjugate()
    QuatSoa QuatSoa::GetInverse() const
    {
        QuatSoa result;
        GetInverse( result );

        return result;
    }

    /// Set this quaternion to its inverse.
    ///
    /// @see GetInverse(), GetConjugate(), SetConjugate()
    void QuatSoa::Invert()
    {
        GetInverse( *this );
    }

    /// Get the conjugate of this quaternion.
    ///
    /// @param[out] rQuat  Quaternion conjugate.
    ///
    /// @see SetConjugate(), GetInverse(), Invert()
    void QuatSoa::GetConjugate( QuatSoa& rQuat ) const
    {
        SimdVector signFlip = Simd::SetSplatU32( 0x80000000 );

        rQuat.m_x = Simd::Xor( m_x, signFlip );
        rQuat.m_y = Simd::Xor( m_y, signFlip );
        rQuat.m_z = Simd::Xor( m_z, signFlip );
        rQuat.m_w = m_w;
    }

    /// Get the conjugate of this quaternion.
    ///
    /// @return  Quaternion conjugate.
    ///
    /// @see SetConjugate(), GetInverse(), Invert()
    QuatSoa QuatSoa::GetConjugate() const
    {
        QuatSoa result;
        GetConjugate( result );

        return result;
    }

    /// Set this quaternion to its conjugate.
    ///
    /// @see GetConjugate(), GetInverse(), Invert()
    void QuatSoa::SetConjugate()
    {
        GetConjugate( *this );
    }

    /// Test whether each component in this quaternion is equal to the corresponding component in another quaternion
    /// within a given threshold.
    ///
    /// @param[in] rQuat     Quaternion.
    /// @param[in] rEpsilon  Comparison threshold.
    ///
    /// @return  SIMD mask with bits set for quaternions that are equal within the given threshold.
    SimdMask QuatSoa::Equals( const QuatSoa& rQuat, const SimdVector& rEpsilon ) const
    {
        SimdVector differenceSquaredX = Simd::SubtractF32( m_x, rQuat.m_x );
        SimdVector differenceSquaredY = Simd::SubtractF32( m_y, rQuat.m_y );
        SimdVector differenceSquaredZ = Simd::SubtractF32( m_z, rQuat.m_z );
        SimdVector differenceSquaredW = Simd::SubtractF32( m_w, rQuat.m_w );

        differenceSquaredX = Simd::MultiplyF32( differenceSquaredX, differenceSquaredX );
        differenceSquaredY = Simd::MultiplyF32( differenceSquaredY, differenceSquaredY );
        differenceSquaredZ = Simd::MultiplyF32( differenceSquaredZ, differenceSquaredZ );
        differenceSquaredW = Simd::MultiplyF32( differenceSquaredW, differenceSquaredW );

        SimdVector epsilonSquared = Simd::MultiplyF32( rEpsilon, rEpsilon );

        SimdMask thresholdMaskX = Simd::LessEqualsF32( differenceSquaredX, epsilonSquared );
        SimdMask thresholdMaskY = Simd::LessEqualsF32( differenceSquaredY, epsilonSquared );
        SimdMask thresholdMaskZ = Simd::LessEqualsF32( differenceSquaredZ, epsilonSquared );
        SimdMask thresholdMaskW = Simd::LessEqualsF32( differenceSquaredW, epsilonSquared );

        return Simd::MaskAnd(
            Simd::MaskAnd( Simd::MaskAnd( thresholdMaskX, thresholdMaskY ), thresholdMaskZ ),
            thresholdMaskW );
    }

    /// Test whether any component in this quaternion is not equal to the corresponding component in another quaternion
    /// within a given threshold.
    ///
    /// @param[in] rQuat     Quaternion.
    /// @param[in] rEpsilon  Comparison threshold.
    ///
    /// @return  SIMD mask with bits set for quaternions that are not equal within the given threshold.
    SimdMask QuatSoa::NotEquals( const QuatSoa& rQuat, const SimdVector& rEpsilon ) const
    {
        SimdVector differenceSquaredX = Simd::SubtractF32( m_x, rQuat.m_x );
        SimdVector differenceSquaredY = Simd::SubtractF32( m_y, rQuat.m_y );
        SimdVector differenceSquaredZ = Simd::SubtractF32( m_z, rQuat.m_z );
        SimdVector differenceSquaredW = Simd::SubtractF32( m_w, rQuat.m_w );

        differenceSquaredX = Simd::MultiplyF32( differenceSquaredX, differenceSquaredX );
        differenceSquaredY = Simd::MultiplyF32( differenceSquaredY, differenceSquaredY );
        differenceSquaredZ = Simd::MultiplyF32( differenceSquaredZ, differenceSquaredZ );
        differenceSquaredW = Simd::MultiplyF32( differenceSquaredW, differenceSquaredW );

        SimdVector epsilonSquared = Simd::MultiplyF32( rEpsilon, rEpsilon );

        SimdMask thresholdMaskX = Simd::GreaterF32( differenceSquaredX, epsilonSquared );
        SimdMask thresholdMaskY = Simd::GreaterF32( differenceSquaredY, epsilonSquared );
        SimdMask thresholdMaskZ = Simd::GreaterF32( differenceSquaredZ, epsilonSquared );
        SimdMask thresholdMaskW = Simd::GreaterF32( differenceSquaredW, epsilonSquared );

        return Simd::MaskOr(
            Simd::MaskOr( Simd::MaskOr( thresholdMaskX, thresholdMaskY ), thresholdMaskZ ),
            thresholdMaskW );
    }

    /// Perform a component-wise addition of this quaternion and the given quaternion.
    ///
    /// @param[in] rQuat  Quaternion to add.
    ///
    /// @return  Quaternion containing the sums of each component.
    QuatSoa QuatSoa::operator+( const QuatSoa& rQuat ) const
    {
        return Add( rQuat );
    }

    /// Perform a component-wise subtraction of the given quaternion from this quaternion.
    ///
    /// @param[in] rQuat  Quaternion to subtract.
    ///
    /// @return  Quaternion containing the differences of each component.
    QuatSoa QuatSoa::operator-( const QuatSoa& rQuat ) const
    {
        return Subtract( rQuat );
    }

    /// Perform a quaternion multiplication between this quaternion and another quaternion.
    ///
    /// @param[in] rQuat  Quaternion with which to multiply.
    ///
    /// @return  Product quaternion.
    QuatSoa QuatSoa::operator*( const QuatSoa& rQuat ) const
    {
        return Multiply( rQuat );
    }

    /// Perform a component-wise in-place addition of this quaternion and the given quaternion.
    ///
    /// @param[in] rQuat  Quaternion to add.
    ///
    /// @return  Reference to this quaternion.
    QuatSoa& QuatSoa::operator+=( const QuatSoa& rQuat )
    {
        AddSet( *this, rQuat );

        return *this;
    }

    /// Perform a component-wise in-place subtraction of the given quaternion from this quaternion.
    ///
    /// @param[in] rQuat  Quaternion to subtract.
    ///
    /// @return  Reference to this quaternion.
    QuatSoa& QuatSoa::operator-=( const QuatSoa& rQuat )
    {
        SubtractSet( *this, rQuat );

        return *this;
    }

    /// Perform in-place multiplication of this quaternion and the given quaternion.
    ///
    /// @param[in] rQuat  Quaternion with which to multiply.
    ///
    /// @return  Reference to this quaternion.
    QuatSoa& QuatSoa::operator*=( const QuatSoa& rQuat )
    {
        MultiplySet( *this, rQuat );

        return *this;
    }

    /// Test whether each component in this quaternion is equal to the corresponding component in another quaternion
    /// within a default threshold.
    ///
    /// @param[in] rQuat  Quaternion.
    ///
    /// @return  SIMD mask with bits set for quaternions that are equal within the given threshold.
    SimdMask QuatSoa::operator==( const QuatSoa& rQuat ) const
    {
        return Equals( rQuat );
    }

    /// Test whether any component in this quaternion is not equal to the corresponding component in another quaternion
    /// within a default threshold.
    ///
    /// @param[in] rQuat  Quaternion.
    ///
    /// @return  SIMD mask with bits set for quaternions that are not equal within the given threshold.
    SimdMask QuatSoa::operator!=( const QuatSoa& rQuat ) const
    {
        return NotEquals( rQuat );
    }
}
