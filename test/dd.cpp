#include "test.h"
#include "rtp.h"
#include "bitstream.h"

class RTPTestPlan: public TestPlan
{
public:
	RTPTestPlan() : TestPlan("Dependency descriptor test plan")
	{
	}
	
	int init() 
	{
		Log("DD::Init\n");
		return true;
	}
	
	int end()
	{
		Log("DD::End\n");
		return true;
	}
	
	
	virtual void Execute()
	{
		init();
		
		Log("testBitreader\n");
		testBitreader();
		
		Log("testParser\n");
		testParser();
		
		Log("Serialize+Parser\n");
		testSerializeParser();
		
		end();
	}
	
	virtual void testBitreader()
	{
		BYTE buffer[1];
		/* n=5
		Value	Full binary encoding	ns(n) encoding
		0	000	                00
		1	001	                01
		2	010	                10
		3	011	                110
		4	100	                111
		*/
		
		BitWritter w(buffer,1);
		BitReader r(buffer,1);
		BYTE val;
		
		w.WriteNonSymmetric(5,0); w.Flush();
		val = r.GetNonSymmetric(5);
		assert(buffer[0]== 0b00000000);
		assert(val==0);
		w.Reset();
		r.Reset();
		
		
		w.WriteNonSymmetric(5,1); w.Flush();
		val = r.GetNonSymmetric(5);
		assert(buffer[0]== 0b01000000);
		assert(val==1);
		w.Reset();
		r.Reset();
		
		w.WriteNonSymmetric(5,2); w.Flush();
		val = r.GetNonSymmetric(5);
		assert(buffer[0]== 0b10000000);
		assert(val==2);
		w.Reset();
		r.Reset();
		
		w.WriteNonSymmetric(5,3); w.Flush();
		val = r.GetNonSymmetric(5);
		assert(buffer[0]== 0b11000000);
		assert(val==3);
		w.Reset();
		r.Reset();
		
		w.WriteNonSymmetric(5,4); w.Flush();
		val = r.GetNonSymmetric(5);
		assert(buffer[0]== 0b11100000);
		assert(val==4);
		w.Reset();
		r.Reset();
				
		
	}
	
	void testSerializeParser()
	{
		BYTE buffer[256];
		BitWritter writter(buffer,sizeof(buffer));
		
		
		DependencyDescriptor dd;
		dd.templateDependencyStructure = TemplateDependencyStructure{};
		dd.templateDependencyStructure->dtisCount = 2;
		dd.templateDependencyStructure->chainsCount = 2;
		dd.templateDependencyStructure->frameDependencyTemplates.emplace_back(FrameDependencyTemplate{
			{0, 0}, 
			{DecodeTargetIndication::Switch, DecodeTargetIndication::Required},
			{1},
			{2,2}
		});
		dd.templateDependencyStructure->decodeTargetProtectedByChain = {0,0};
		
		
		assert(dd.Serialize(writter));
			
		auto len = writter.Flush();
		
		BitReader reader(buffer,len);
		
		auto parsed = DependencyDescriptor::Parse(reader);
		Dump(buffer,len);
		dd.Dump();
		assert(parsed);
		parsed->Dump();
		assert(dd == parsed.value());
		
		
		writter.Reset();
		DependencyDescriptor dd2;
		dd2.customDecodeTargetIndications	= std::vector<DecodeTargetIndication>{DecodeTargetIndication::Switch, DecodeTargetIndication::Required};
		dd2.customFrameDiffs			= std::vector<uint32_t>{1};
		dd2.customFrameDiffsChains		= std::vector<uint32_t>{2,2};
		
		assert(dd2.Serialize(writter));
		
		len = writter.Flush();
		::Dump(buffer,len);
		
		
	}
	
	void testParser()
	{
		BYTE data[10] = { 0x80,   0x00,   0x01,   0x80,   0x00,   0xe2,   0x04,   0xfe, 0x03,   0xbe};
		BitReader reader(data,10);
		auto dd = DependencyDescriptor::Parse(reader);
		
		assert(dd->templateDependencyStructure->ContainsFrameDependencyTemplate(0));
		dd->Dump();

	}
		
};

RTPTestPlan dd;
