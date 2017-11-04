/*
	(c) Citadel Software Inc, 2003, www.citadelsoftware.ca
*/

// Compute the machine signature
#pragma once

namespace CitadelSoftwareInc {


	/*! \brief Get a hash of quantities that will provide a machine signature, such as mac address, current process info etc.
	*
	* This is used after the seed file is processed to add something diffent to the pool hashes stored in the 
	* machine file to avoid provlems with reusing the same seed file.
	*/
	void GetMachineSignature(vecuc& vData);



}	// end namespace CitadelSoftwareInc