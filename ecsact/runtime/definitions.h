#ifndef ECSACT_DEFINITIONS_H
#define ECSACT_DEFINITIONS_H

#include <stdint.h>
#include <stdbool.h>
#include "ecsact/runtime/common.h"

/**
 * Built-in Ecsact types.
 *
 * @details
 *   Built-in type identifier is composed of 4 bytes. These details are not
 *   necessary to understand in order to use this enum, but may provide insight
 *   on the values set.
 *
 *   byte 1 signifies the kind of built-in type
 *     0000 - unsigned integer
 *     1000 - signed integer
 *     0100 - floating point
 *     0010 - entity ID
 *   byte 2 through 4 represents the size in bits
 *     Although this implies the bit size of a type can be up to 255^3, only the
 *     values specified in the `ecsact_builtin_type` enum are valid.
 */
typedef enum ecsact_builtin_type {
	ECSACT_BOOL = 0b0000000000000001,
	ECSACT_I8 = 0b1000000000001000,
	ECSACT_U8 = 0b0000000000001000,
	ECSACT_I16 = 0b1000000000010000,
	ECSACT_U16 = 0b0000000000010000,
	ECSACT_I32 = 0b1000000000100000,
	ECSACT_U32 = 0b0000000000100000,
	ECSACT_F32 = 0b0100000000100000,
	ECSACT_ENTITY_TYPE = 0b0010000000100000,
} ecsact_builtin_type;

typedef enum ecsact_type_kind {
	ECSACT_TYPE_KIND_BUILTIN,
	ECSACT_TYPE_KIND_ENUM,
} ecsact_type_kind;

typedef union ecsact_type {
	ecsact_builtin_type builtin;
	ecsact_enum_id      enum_id;
} ecsact_type;

typedef struct ecsact_field_type {
	ecsact_type_kind kind;
	ecsact_type      type;

	/**
	 * equals 1        single field (not fixed array)
	 * greater-than 1  fixed array of that length
	 * equals 0        invalid - reserved for future use
	 */
	int32_t length;
} ecsact_field_type;

typedef struct ecsact_field_definitions {
	/**
	 * Name of field. Null-terminated string. May be empty.
	 */
	char* name;

	/**
	 * Type of field.
	 */
	ecsact_field_type type;
} ecsact_field_definition;

typedef struct ecsact_component_definition {
	/**
	 * Name of component. Null-terminated string. May be empty.
	 */
	char* name;

	/**
	 * Component only exists temporarily during system execution.
	 */
	bool transient;

	/**
	 * Length of `fields` list.
	 */
	int32_t fields_len;

	/**
	 * Array of component fields. Number of elements specified by `fields_len`.
	 */
	ecsact_field_definition* fields;
} ecsact_component_definition;

typedef struct ecsact_system_definition {
	char*                     name;
	int32_t                   capabilities_len;
	ecsact_component_id*      capability_components;
	ecsact_system_capability* capabilities;
	int32_t                   child_systems_len;
	ecsact_system_id*         child_systems;
} ecsact_system_definition;

#endif // ECSACT_DEFINITIONS_H
