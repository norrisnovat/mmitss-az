/*
 * Generated by asn1c-0.9.21 (http://lionet.info/asn1c)
 * From ASN.1 module "J2735MAPMESSAGE"
 * 	found in "MAP.txt"
 * 	`asn1c -fnative-types`
 */

#ifndef	_Approach_H_
#define	_Approach_H_


#include <asn_application.h>

/* Including external dependencies */
#include "DescriptiveName.h"
#include "ApproachNumber.h"
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct VehicleReferenceLane;
struct VehicleComputedLane;
struct SpecialLane;
struct BarrierLane;
struct CrosswalkLane;

/* Approach */
typedef struct Approach {
	DescriptiveName_t	*name	/* OPTIONAL */;
	ApproachNumber_t	*id	/* OPTIONAL */;
	struct drivingLanes {
		A_SEQUENCE_OF(struct VehicleReferenceLane) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} drivingLanes;
	struct computedLanes {
		A_SEQUENCE_OF(struct VehicleComputedLane) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} computedLanes;
	struct trainsAndBuses {
		A_SEQUENCE_OF(struct SpecialLane) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} trainsAndBuses;
	struct barriers {
		A_SEQUENCE_OF(struct BarrierLane) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} barriers;
	struct crosswalks {
		A_SEQUENCE_OF(struct CrosswalkLane) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} crosswalks;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Approach_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Approach;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "VehicleReferenceLane.h"
#include "VehicleComputedLane.h"
#include "SpecialLane.h"
#include "BarrierLane.h"
#include "CrosswalkLane.h"

#endif	/* _Approach_H_ */
