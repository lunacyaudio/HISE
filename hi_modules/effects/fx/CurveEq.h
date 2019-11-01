/*  ===========================================================================
*
*   This file is part of HISE.
*   Copyright 2016 Christoph Hart
*
*   HISE is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   HISE is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with HISE.  If not, see <http://www.gnu.org/licenses/>.
*
*   Commercial licenses for using HISE in an closed source project are
*   available on request. Please visit the project's website to get more
*   information about commercial licensing:
*
*   http://www.hise.audio/
*
*   HISE is based on the JUCE library,
*   which must be separately licensed for closed source applications:
*
*   http://www.juce.com
*
*   ===========================================================================
*/

#ifndef CURVEEQ_H_INCLUDED
#define CURVEEQ_H_INCLUDED

namespace hise { using namespace juce;

#define FFT_SIZE_FOR_EQ 4096

#define OLD_EQ_FFT 0

/** Set this to true to use the new SVF EQs which sound better when modulated. 

	By default it uses the stock biquad filters from JUCE for backward-compatibility but this value might be changed
	some time in the future
*/
#ifndef HISE_USE_SVF_FOR_CURVE_EQ
#define HISE_USE_SVF_FOR_CURVE_EQ 0
#endif

/** A parametriq equalizer with unlimited bands and FFT display. 
*	@ingroup effectTypes
*
*/
class CurveEq: public MasterEffectProcessor
{
public:

	SET_PROCESSOR_NAME("CurveEq", "Parametriq EQ", "A parametric EQ with a variable amount of filter bands.")

	enum Parameters
	{
		numEffectParameters
	};

	/** The types of filters for every band. */
	enum FilterType
	{
		LowPass = 0, ///< a 1-Pole Lowpass
		HighPass, ///< a 1-Pole Highpass
		LowShelf, ///< a shelving eq for the low end
		HighShelf, ///< a shelving eq for the high end
		Peak, ///< a peak eq
		numFilterTypes
	};

	/** The parameters for each band. */
	enum BandParameter
	{
		Gain = 0, ///< the gain (not available on HP/LP)
		Freq, ///< the center frequency
		Q, ///< the q factor (not available on HP/LP)
		Enabled, ///< enables / disables the band
		Type, ///< defines the type of the band @see FilterType
		numBandParameters
	};

#if HISE_USE_SVF_FOR_CURVE_EQ
	using FilterTypeForEq = StateVariableEqSubType;
#else
	using FilterTypeForEq = StaticBiquadSubType;
#endif

	struct StereoFilter : public MultiChannelFilter<FilterTypeForEq>
	{
		StereoFilter()
		{
			setNumChannels(2);
			setSmoothingTime(0.015);
		}

		void renderIfEnabled(FilterHelpers::RenderData& r)
		{
			if (enabled)
				render(r);
		}

		void setEnabled(bool shouldBeEnabled)
		{
			enabled = shouldBeEnabled;
		}

		bool isEnabled() const
		{
			return enabled;
		}

	private:

		bool enabled = true;
	};

	CurveEq(MainController *mc, const String &id):
		MasterEffectProcessor(mc, id)
	{
		finaliseModChains();

		parameterNames.add("Gain");			
		parameterDescriptions.add("The gain in decibels if supported from the filter type.");

		parameterNames.add("Freq");
		parameterDescriptions.add("The frequency in Hz.");

		parameterNames.add("Q");
		parameterDescriptions.add("The bandwidth of the filter if supported.");

		parameterNames.add("Enabled");
		parameterDescriptions.add("the state of the filter band.");

		parameterNames.add("Type");
		parameterDescriptions.add("the filter type of the filter band.");

		parameterNames.add("BandOffset");
		parameterDescriptions.add("the offset that can be used to get the desired formula.");

	};

	int getParameterIndex(int filterIndex, int parameterType) const
	{
		return filterIndex * numBandParameters + parameterType;
	}

	float getAttribute(int index) const override;;

	void setInternalAttribute(int index, float newValue) override;;

	const AnalyserRingBuffer& getFFTBuffer() const
	{
		return fftBuffer;
	}

	void applyEffect(AudioSampleBuffer &buffer, int startSample, int numSamples) override
	{
		FilterHelpers::RenderData r(buffer, startSample, numSamples);

		for (auto filter : filterBands)
		{
			filter->renderIfEnabled(r);
		}

		if (fftBuffer.isActive())
			fftBuffer.pushSamples(buffer, startSample, numSamples);

#if OLD_EQ_FFT
		if(fftBufferIndex < FFT_SIZE_FOR_EQ)
		{
			const int numSamplesToCopy = jmin<int>(numSamples, FFT_SIZE_FOR_EQ - fftBufferIndex);

			FloatVectorOperations::copyWithMultiply(fftData + fftBufferIndex, buffer.getReadPointer(0, startSample), 0.5f, numSamplesToCopy);
			FloatVectorOperations::addWithMultiply(fftData + fftBufferIndex, buffer.getReadPointer(1, startSample), 0.5f, numSamplesToCopy);

			fftBufferIndex += numSamplesToCopy;

		}
#endif
	};

	IIRCoefficients getCoefficients(int filterIndex)
	{
		return filterBands[filterIndex]->getApproximateCoefficients();
	};

	int getNumFilterBands() const
	{
		return filterBands.size();
	};

	StereoFilter *getFilterBand(int filterIndex)
	{
		return filterBands[filterIndex];
	}

	const StereoFilter *getFilterBand(int filterIndex) const
	{
		return filterBands[filterIndex];
	}

	void enableSpectrumAnalyser(bool shouldBeEnabled)
	{
		fftBuffer.setAnalyserBufferSize(shouldBeEnabled ? 16384 : 0);
	}

	void addFilterBand(double freq, double gain)
	{
		ScopedLock sl(getMainController()->getLock());

		StereoFilter *f = new StereoFilter();

		f->setSampleRate(getSampleRate());
		f->setType(CurveEq::Peak);
		f->setGain(gain);
		f->setFrequency(freq);

		filterBands.add(f);

		sendChangeMessage();
	}

	void removeFilterBand(int filterIndex)
	{
		ScopedLock sl(getMainController()->getLock());

		filterBands.remove(filterIndex);

		sendChangeMessage();
	}

	void prepareToPlay(double sampleRate, int samplesPerBlock) override
	{
		ScopedLock sl(getMainController()->getLock());

		MasterEffectProcessor::prepareToPlay(sampleRate, samplesPerBlock);

		if (lastSampleRate != sampleRate)
		{
			lastSampleRate = sampleRate;

			for (int i = 0; i < filterBands.size(); i++)
			{
				filterBands[i]->setSampleRate(sampleRate);
			}
		}
	};

	ValueTree exportAsValueTree() const override
	{
		ValueTree v = MasterEffectProcessor::exportAsValueTree();

		v.setProperty("NumFilters", filterBands.size(), nullptr);

		for(int i = 0; i < filterBands.size() * numBandParameters; i++)
		{
			v.setProperty("Band" + String(i), getAttribute(i), nullptr);
		}

		v.setProperty("FFTEnabled", fftBuffer.isActive(), nullptr);

		return v;
	};

	void restoreFromValueTree(const ValueTree &v) override
	{
		MasterEffectProcessor::restoreFromValueTree(v);

		ScopedLock sl(getMainController()->getLock());

		filterBands.clear();

		const int numFilters = v.getProperty("NumFilters", 0);

		for(int i = 0; i < numFilters; i++)
		{
			filterBands.add(new StereoFilter());
		}

		for(int i = 0; i < numFilters * numBandParameters; i++)
		{
            const float value = v.getProperty("Band" + String(i), 0.0f);
            setAttribute(i, value, dontSendNotification);
		}

		enableSpectrumAnalyser(v.getProperty("FFTEnabled", false));

		sendSynchronousChangeMessage();
	}

	struct AlignedDouble
	{
		double data;
		double padding;
	};

	bool hasTail() const override {return false;};

	int getNumChildProcessors() const override { return 0; };

	Processor *getChildProcessor(int /*processorIndex*/) override { return nullptr; };

	const Processor *getChildProcessor(int /*processorIndex*/) const override { return nullptr; };

	ProcessorEditorBody *createEditor(ProcessorEditor *parentEditor)  override;

private:

	AnalyserRingBuffer fftBuffer;

#if OLD_EQ_FFT
	float fftData[FFT_SIZE_FOR_EQ];
	double externalFftData[FFT_SIZE_FOR_EQ];
	int fftBufferIndex;
#endif

	OwnedArray<StereoFilter> filterBands;
	
	double lastSampleRate = 0.0;

	JUCE_DECLARE_WEAK_REFERENCEABLE(CurveEq);
};

} // namespace hise


#endif  // CURVEEQ_H_INCLUDED
