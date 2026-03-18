/**
 * @brief Optimized IQ imbalance blind compensation.
 *
 * Performs: DC removal, gain balancing, and linear decorrelation
 * in minimal passes over data (2 instead of 6).
 *
 * @param signal_data Pointer to IQ signal structure.
 */
void iq_compensation(signal_iq_t *signal_data)
{
    if (!signal_data || !signal_data->signal_iq || signal_data->n_signal == 0)
        return;

    const size_t N = signal_data->n_signal;
    double complex *x = signal_data->signal_iq;
    const double eps = 1e-20;

    double meanI = 0.0, meanQ = 0.0, pI = 0.0, pQ = 0.0, crossIQ = 0.0;

    /* Pass 1: Compute DC means and branch powers in single loop */
#pragma omp parallel for reduction(+ : meanI, meanQ, pI, pQ)
    for (size_t n = 0; n < N; n++)
    {
        const double I_n = creal(x[n]);
        const double Q_n = cimag(x[n]);
        meanI += I_n;
        meanQ += Q_n;
        pI += I_n * I_n;
        pQ += Q_n * Q_n;
    }

    meanI /= N;
    meanQ /= N;

    if (pI <= eps || pQ <= eps)
        return;

    /* Pass 2: DC removal, gain balance, and cross-correlation in one loop */
    const double gain = sqrt(pI / pQ);
    pI = 0.0; /* Reuse for normalized power */

#pragma omp parallel for reduction(+ : pI, crossIQ)
    for (size_t n = 0; n < N; n++)
    {
        const double I_n = creal(x[n]) - meanI;
        const double Q_n = (cimag(x[n]) - meanQ) * gain;
        x[n] = I_n + I * Q_n;
        pI += I_n * I_n;
        crossIQ += I_n * Q_n;
    }

    /* Pass 3: Decorrelation (final step) */
    const double rho = crossIQ / (pI + eps);

#pragma omp parallel for
    for (size_t n = 0; n < N; n++)
    {
        const double I_n = creal(x[n]);
        x[n] = I_n + I * (cimag(x[n]) - rho * I_n);
    }
}