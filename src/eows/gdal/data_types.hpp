#ifndef __EOWS_GDAL_DATA_TYPES_HPP__
#define __EOWS_GDAL_DATA_TYPES_HPP__

#include "exception.hpp"

#include "../geoarray/data_types.hpp"

#include <gdal_priv.h>

namespace eows
{
  namespace gdal
  {
    /*!
     * \brief Defines a EOWS Raster Band property containing metadata information like dummy value, eows data type, etc.
     */
    struct property
    {
      std::size_t index; //!< Property index (same band)
      int dtype;         //!< Band datatype definition
      double dummy;      //!< Dummy value for band
      int width;         //!< Band width
      int height;        //!< Band height

      property(const std::size_t i, int t)
        : index(i), dtype(t), dummy(-9999), width(0), height(0)
      {
      }

      /*!
       * \brief Retrieves a property data type from GDALDataType
       * \param dt - GDAL datatype
       * \return Band datatype
       */
      static int from_gdal_datatype(GDALDataType dt)
      {
        switch(dt)
        {
          case GDT_Byte:
            return eows::geoarray::datatype_t::int8_dt;
          case GDT_Int16:
            return eows::geoarray::datatype_t::int16_dt;
          case GDT_UInt16:
            return eows::geoarray::datatype_t::uint16_dt;
          case GDT_Int32:
            return eows::geoarray::datatype_t::int32_dt;
          case GDT_UInt32:
            return eows::geoarray::datatype_t::uint32_dt;
          case GDT_Float32:
            return eows::geoarray::datatype_t::float_dt;
          case GDT_Float64:
            return eows::geoarray::datatype_t::double_dt;
          default:
          {
            throw gdal_error("Invalid GDAL datatype");
          }
        }
      }
      /*!
       * \brief Retrieves a GDALDataType from EOWS GDAL datatype
       * \throws eows::gdal::gdal_error When a datatype does not exists or not supported yet.
       * \param dt - EOWS GDAL data type
       * \return GDAL data type
       */
      static GDALDataType from_datatype(int dt)
      {
        switch(dt)
        {
          case eows::geoarray::datatype_t::int8_dt:
            return GDT_Byte;
          case eows::geoarray::datatype_t::int16_dt:
            return GDT_Int16;
          case eows::geoarray::datatype_t::uint16_dt:
            return GDT_UInt16;
          case eows::geoarray::datatype_t::int32_dt:
            return GDT_Int32;
          case eows::geoarray::datatype_t::uint32_dt:
            return GDT_Int32;
          case eows::geoarray::datatype_t::float_dt:
            return GDT_Float32;
          case eows::geoarray::datatype_t::double_dt:
            return GDT_Float64;
          default:
          {
            throw gdal_error("Invalid datatype");
          }
        }
      }
    };
  } //! end namespace gdal
}   //! end namespace eows

#endif //__EOWS_GDAL_DATA_TYPES_HPP__
