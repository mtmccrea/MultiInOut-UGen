/*
    The MultiInOut UGen exemplifies reading a variable number of input channels
    and generating multiple channels of output.
    In order to focus on this topic, input and output rates are limited to
    audio-rate, and parameters (modFreq and modDepth) are not modulatable,
    i.e. "scalar-rate" or "initialization-rate", for brevity and simplicity.
    A fully-featured UGen would support audio- and control-rate output, as well
    as modulatable input parameters (at various rates).
*/
#include "SC_PlugIn.hpp"

// InterfaceTable contains pointers to functions in the host (server).
static InterfaceTable *ft;

// declare struct to hold unit generator state, inherit from SCUnit
struct MultiInOut : public SCUnit {

public:
    /*
        Constructor: usually does these things.
        1. set the calculation function.
        2. initialize the UGen state variables.
        3. allocate any memory needed by the UGen.
        4. calculate an "initialization sample" of output.
          (for initializing downstream UGens.)
        5. reset the state of the UGen to prepare for generating output.
          (first sample of output should be equal to the intialization sample.)
    */
    MultiInOut() {
        /*
            1. Set the calculation function.
        */
        // NOTE: set_calc_function will also call your calc function, which
        // we want to do later, so set mCalcFunc directly
        mCalcFunc = make_calc_function<MultiInOut,&MultiInOut::next_a>();

        /*
            2. Initialize UGen state variables.
              (Use the first frame of UGen's inputs, where needed.)
        */
        // Initialize mPhaseInc by using the initial value (first frame) of
        // 'modFreq' input to calculate the phase increment.
        mPhaseInc = in0(0) * twopi * sampleDur();

        // In this example, modFreq and modDepth are non-modulatable
        // for simplicity, so just store result to member variable.
        mModMul = in0(1) * 0.5;   // 'modDepth' is the second input
        mModAdd = 1.0 - mModMul;  // normalize peak to 1

        // Store the number of audio input/output channels to process.
        // In this example, the number of output channels is equal to the
        // size of the 'inputArray' argument.
        mNumOutputChans = numInputs() - 2;  // - 2: exclude modFreq, modDepth

        /*
            3. Memory allocation.
        */
        // The input is modulated by a sinusoidal amplitude function.
        // The phase of each channel's modulator is offset to create an
        // amplitude "sweep" effect. Store these phase offsets for each channel.
        mPhaseOffsets = nullptr; // in case we error out before assigning this
        mPhaseOffsets = (float*)RTAlloc(mWorld, mNumOutputChans * sizeof(double));

        // This check makes sure that RTAlloc succeeded. (It might fail if there's not enough memory.)
        // If you don't do this check properly then YOU CAN CRASH THE SERVER!
        // A lot of ugens in core and sc3-plugins fail to do this. Don't follow their example.
        if (mPhaseOffsets == nullptr) {
        // if (mIns == nullptr || mOuts == nullptr || mPhaseOffsets == nullptr) {
            // calc function should now simply clear the outputs
            // mCalcFunc = ft->fClearUnitOutputs; // SETCALC
            mCalcFunc = *ClearUnitOutputs; // SETCALC
            ClearUnitOutputs(this, 1);

            if (mWorld->mVerbosity > -2) {
                Print("Failed to allocate memory for MultiInOut UGen.\n");
            }

            mDone = true; // should this be here?
            return;
        }

        // initial phase of the base modulator
        mPhase = 0.;

        // Populate the mPhaseOffsets array with the constant
        // phase offsets from the base modulator, for each channel.
        double phaseStep = twopi / mNumOutputChans;
        for (int i = 0; i < mNumOutputChans; i++) {
            mPhaseOffsets[i] = i * phaseStep;
        }

        /*
            4. Calculate one "initialization sample" of output
        */
        // Call the calculation function with 1 sample for initializing
        // downstream UGens properly. (This sample isn't actually output!)
        next_a(1);

        /*
            5. Reset the state of the UGen to prepare for generating output.
        */
        // Reset the phase to "undo" phase advancement done in next_a(1).
        // This is so that the first output sample will be the same as the
        // "initialization sample". If your calculation function alters other
        // state variables, reset those too.
        mPhase = 0.;
    }

    /*
        Destructor: free memory allocated in the the UGen's constructor
    */
    ~MultiInOut() {
        // free memory resources
        RTFree(mWorld, mPhaseOffsets);
    }

private:
    /*
        Declare your member (state) variables, which store the UGen's state from
        one block to the next.
    */
    int mNumInputChans, mNumOutputChans;
    // const float **mIns;
    // float **mOuts;
    float *mPhaseOffsets;
    double mPhase, mPhaseInc;
    float mModMul, mModAdd;
    //////////////////////////////////////////////////////////////////

    // calculation function for an audio rate frequency argument
    void next_a(int inNumSamples)
    {
        /*
            Store UGen's member variables locally for efficient access,
            setting, and iteration.
        */
        float modMul = mModMul;
        float modAdd = mModAdd;
        double phaseInc = mPhaseInc;
        double phase = mPhase;
        float *phaseOffsets = mPhaseOffsets;
        int numOutputs = mNumOutputChans;

        // The Unit struct has members: float **mInBuf, **mOutBuf, which are
        // pointers to the location (head) of each input and output channel,
        // respectively. This allows you to access the samples of the inputs,
        // for example, in the format: mInBuf[channel][frame].
        // Here, we offset the input pointer directly to the 'inputArray' input
        // (skipping the other 2 arg inputs) so we can use zero-based indexing
        // in the sample loop below, matching the output channels indices.
        // Pointer arithmetic works here to perform that offset.
        float **inBuf = mInBuf + 2;   // + 2: skip modFreq an modDepth inputs
        float **outBuf = mOutBuf;     // pointer to buffer pointers

        /*
            Perform a loop over the control block's frames.
            If this UGen is audio-rate, then inNumSamples will be 64 (the
            default, unless the user has set a different block size). If this
            unit is control-rate, then inNumSamples will be 1.
        */
        for (int frm = 0; frm < inNumSamples; frm++) {  // iterate over each frame in this block

            for (int chan = 0; chan < numOutputs; chan++) {  // iterate over each channel in this frame
                // calculate the phase for each modulator
                double thisPhase = phase + phaseOffsets[chan];

                // generate modulation from a sine function (not very efficient)
                float mod = sin(thisPhase) * modMul + modAdd;

                // output = input * modulation
                outBuf[chan][frm] = inBuf[chan][frm] * mod;
            }
            phase += phaseInc;
        }

        /*
            Store local variable states back to the the struct's member
            variables to carry over to the next block.
        */
        mPhase = (double)sc_wrap(phase, 0.0, twopi);
    }
};

// The entry point is called by the host when the plug-in is loaded.
PluginLoad(MultiInOut)
{
    // InterfaceTable *inTable implicitly given as argument to the load function
    ft = inTable; // store pointer to InterfaceTable

    // registerUnit takes the place of the Define*Unit functions. It automatically checks for the presence of a
    // destructor function.
    // However, it does not seem to be possible to disable buffer aliasing with the C++ header.
    registerUnit<MultiInOut>(ft, "MultiInOut");
}
