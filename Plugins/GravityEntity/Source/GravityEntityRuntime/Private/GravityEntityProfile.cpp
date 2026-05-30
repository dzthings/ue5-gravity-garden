#include "GravityEntityProfile.h"
#include "GravityPartGenerator.h"

UGravityPartGenerator* UGravityEntityProfile::FindGeneratorForRole(EGravityNodeRole Role) const
{
	for (const TObjectPtr<UGravityPartGenerator>& Gen : PartGenerators)
	{
		if (Gen && Gen->Roles.Contains(Role))
		{
			return Gen.Get();
		}
	}
	return nullptr;
}
